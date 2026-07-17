// Export instructions that use one or more requested scalar operands.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.scalar.Scalar;

import java.io.File;
import java.io.PrintWriter;
import java.util.LinkedHashSet;
import java.util.Set;

public class TraceScalarOperands extends GhidraScript {
    private static String csv(String value) {
        return "\"" + value.replace("\"", "\"\"") + "\"";
    }

    private static long parseOffset(String value) {
        String normalized = value.trim().toLowerCase();
        if (normalized.startsWith("0x")) {
            return Long.parseUnsignedLong(normalized.substring(2), 16);
        }
        return Long.parseLong(normalized);
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length < 2) {
            throw new IllegalArgumentException(
                "Expected an output CSV followed by one or more scalar operands");
        }

        File output = new File(arguments[0]);
        File parent = output.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            throw new IllegalStateException("Unable to create " + parent);
        }

        Set<Long> requested = new LinkedHashSet<>();
        for (int index = 1; index < arguments.length; ++index) {
            requested.add(parseOffset(arguments[index]));
        }

        Listing listing = currentProgram.getListing();
        int matchCount = 0;
        try (PrintWriter writer = new PrintWriter(output, "UTF-8")) {
            writer.println(
                "functionAddress,functionName,instructionAddress,mnemonic,instruction,offsets");
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext() && !monitor.isCancelled()) {
                Function function = functions.next();
                InstructionIterator instructions = listing.getInstructions(function.getBody(), true);
                while (instructions.hasNext() && !monitor.isCancelled()) {
                    Instruction instruction = instructions.next();
                    Set<Long> matches = new LinkedHashSet<>();
                    for (int operand = 0; operand < instruction.getNumOperands(); ++operand) {
                        for (Object object : instruction.getOpObjects(operand)) {
                            if (object instanceof Scalar) {
                                long value = ((Scalar)object).getUnsignedValue();
                                if (requested.contains(value)) {
                                    matches.add(value);
                                }
                            }
                        }
                    }
                    if (matches.isEmpty()) {
                        continue;
                    }

                    StringBuilder offsets = new StringBuilder();
                    for (long match : matches) {
                        if (offsets.length() != 0) {
                            offsets.append(';');
                        }
                        offsets.append(String.format("0x%X", match));
                    }
                    writer.println(
                        csv(function.getEntryPoint().toString()) + "," +
                        csv(function.getName(true)) + "," +
                        csv(instruction.getAddress().toString()) + "," +
                        csv(instruction.getMnemonicString()) + "," +
                        csv(instruction.toString()) + "," +
                        csv(offsets.toString()));
                    ++matchCount;
                }
            }
        }
        println("Exported " + matchCount + " matching instructions to " + output);
    }
}
