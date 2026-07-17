// Recover GameBox class registrations from their static initializer pattern.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class ExportClassRegistry extends GhidraScript {
    private static final class Registration {
        String className;
        long classId;
        String initializer;
        long constructor;
        long descriptor;
        long registry;
        String registerFunction;
    }

    private static String csv(String value) {
        return "\"" + value.replace("\"", "\"\"") + "\"";
    }

    private static boolean mnemonic(Instruction instruction, String expected) {
        return instruction != null && instruction.getMnemonicString().equalsIgnoreCase(expected);
    }

    private static Long scalar(Instruction instruction, int operand) {
        if (instruction == null) {
            return null;
        }
        Scalar scalar = instruction.getScalar(operand);
        return scalar == null ? null : scalar.getUnsignedValue();
    }

    private static String address(long value) {
        return String.format("%08X", value & 0xffffffffL);
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length != 1) {
            throw new IllegalArgumentException("Expected one export-directory argument");
        }

        File output = new File(arguments[0]);
        if (!output.exists() && !output.mkdirs()) {
            throw new IllegalStateException("Unable to create " + output);
        }

        Listing listing = currentProgram.getListing();
        ReferenceManager references = currentProgram.getReferenceManager();
        Set<String> emitted = new HashSet<>();
        List<Registration> registrations = new ArrayList<>();

        DataIterator dataIterator = listing.getDefinedData(true);
        while (dataIterator.hasNext() && !monitor.isCancelled()) {
            Data data = dataIterator.next();
            Object value = data.getValue();
            if (!(value instanceof String)) {
                continue;
            }
            String className = (String)value;
            if (!className.matches("^C[A-Z][A-Za-z0-9_:<>~]*$")) {
                continue;
            }

            ReferenceIterator stringReferences = references.getReferencesTo(data.getAddress());
            while (stringReferences.hasNext() && !monitor.isCancelled()) {
                Reference reference = stringReferences.next();
                Instruction classNamePush = listing.getInstructionAt(reference.getFromAddress());
                if (!mnemonic(classNamePush, "PUSH")) {
                    continue;
                }

                Instruction constructorPush =
                    listing.getInstructionBefore(classNamePush.getAddress());
                Instruction descriptorPush =
                    listing.getInstructionAfter(classNamePush.getAddress());
                Instruction classIdPush = descriptorPush == null ? null :
                    listing.getInstructionAfter(descriptorPush.getAddress());
                Instruction registryMove = classIdPush == null ? null :
                    listing.getInstructionAfter(classIdPush.getAddress());
                Instruction registerCall = registryMove == null ? null :
                    listing.getInstructionAfter(registryMove.getAddress());

                if (!mnemonic(constructorPush, "PUSH") ||
                    !mnemonic(descriptorPush, "PUSH") ||
                    !mnemonic(classIdPush, "PUSH") ||
                    !mnemonic(registryMove, "MOV") ||
                    !mnemonic(registerCall, "CALL")) {
                    continue;
                }

                Long constructor = scalar(constructorPush, 0);
                Long descriptor = scalar(descriptorPush, 0);
                Long classId = scalar(classIdPush, 0);
                Long registry = scalar(registryMove, 1);
                Address[] callFlows = registerCall.getFlows();
                if (constructor == null || descriptor == null || classId == null ||
                    registry == null || callFlows.length != 1 ||
                    (classId & 0xfffL) != 0 || classId < 0x01000000L ||
                    classId > 0x0fffffffL) {
                    continue;
                }

                String key = className + ":" + classId;
                if (!emitted.add(key)) {
                    continue;
                }

                Registration registration = new Registration();
                registration.className = className;
                registration.classId = classId;
                registration.initializer = classNamePush.getAddress().toString();
                registration.constructor = constructor;
                registration.descriptor = descriptor;
                registration.registry = registry;
                registration.registerFunction = callFlows[0].toString();
                registrations.add(registration);
            }
        }

        Map<Long, Long> descriptorClassIds = new HashMap<>();
        for (Registration registration : registrations) {
            descriptorClassIds.put(registration.registry, registration.classId);
        }

        try (PrintWriter writer =
                 new PrintWriter(new File(output, "class-registry.csv"), "UTF-8")) {
            writer.println(
                "classId,className,parentClassId,initializer,constructor,descriptor,registry,registerFunction");
            for (Registration registration : registrations) {
                long parentClassId = descriptorClassIds.getOrDefault(registration.descriptor, 0L);
                writer.println(
                    csv(String.format("0x%08X", registration.classId)) + "," +
                    csv(registration.className) + "," +
                    csv(String.format("0x%08X", parentClassId)) + "," +
                    csv(registration.initializer) + "," +
                    csv("0x" + address(registration.constructor)) + "," +
                    csv("0x" + address(registration.descriptor)) + "," +
                    csv("0x" + address(registration.registry)) + "," +
                    csv(registration.registerFunction));
            }
        }

        println("Recovered " + registrations.size() + " GameBox class registrations");
    }
}
