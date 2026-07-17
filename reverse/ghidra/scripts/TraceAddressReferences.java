// Export references to one or more requested program addresses.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;

import java.io.File;
import java.io.PrintWriter;

public class TraceAddressReferences extends GhidraScript {
    private static String csv(String value) {
        return "\"" + value.replace("\"", "\"\"") + "\"";
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length < 2) {
            throw new IllegalArgumentException(
                "Expected an output CSV followed by one or more program addresses");
        }

        File output = new File(arguments[0]);
        File parent = output.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            throw new IllegalStateException("Unable to create " + parent);
        }

        ReferenceManager references = currentProgram.getReferenceManager();
        int matchCount = 0;
        try (PrintWriter writer = new PrintWriter(output, "UTF-8")) {
            writer.println(
                "targetAddress,fromAddress,functionAddress,functionName,referenceType,instruction");
            for (int index = 1; index < arguments.length; ++index) {
                Address target = toAddr(arguments[index]);
                ReferenceIterator iterator = references.getReferencesTo(target);
                while (iterator.hasNext() && !monitor.isCancelled()) {
                    Reference reference = iterator.next();
                    Address from = reference.getFromAddress();
                    Function function =
                        currentProgram.getFunctionManager().getFunctionContaining(from);
                    Instruction instruction = currentProgram.getListing().getInstructionAt(from);
                    writer.println(
                        csv(target.toString()) + "," +
                        csv(from.toString()) + "," +
                        csv(function == null ? "" : function.getEntryPoint().toString()) + "," +
                        csv(function == null ? "" : function.getName(true)) + "," +
                        csv(reference.getReferenceType().toString()) + "," +
                        csv(instruction == null ? "" : instruction.toString()));
                    ++matchCount;
                }
            }
        }
        println("Exported " + matchCount + " address references to " + output);
    }
}
