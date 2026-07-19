// Export instruction listings for one or more containing functions.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

import java.io.File;
import java.io.PrintWriter;

public class DisassembleAt extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length < 2) {
            throw new IllegalArgumentException(
                "Expected an output directory followed by one or more addresses");
        }

        File output = new File(arguments[0]);
        if (!output.exists() && !output.mkdirs()) {
            throw new IllegalStateException("Unable to create " + output);
        }

        for (int index = 1; index < arguments.length; ++index) {
            Address address = toAddr(arguments[index]);
            Function function = currentProgram.getFunctionManager().getFunctionContaining(address);
            if (function == null) {
                printerr("No function contains " + address);
                continue;
            }

            String filename = function.getEntryPoint() + "_" +
                function.getName().replaceAll("[^A-Za-z0-9_.-]", "_") + ".asm";
            try (PrintWriter writer = new PrintWriter(new File(output, filename), "UTF-8")) {
                InstructionIterator instructions =
                    currentProgram.getListing().getInstructions(function.getBody(), true);
                while (instructions.hasNext() && !monitor.isCancelled()) {
                    Instruction instruction = instructions.next();
                    writer.println(instruction.getAddress() + "  " + instruction);
                }
            }
            println("Disassembled " + function + " to " + filename);
        }
    }
}
