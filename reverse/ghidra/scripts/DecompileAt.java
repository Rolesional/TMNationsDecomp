// Decompile one or more addresses into stable text files.
// @category TMNF

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

import java.io.File;
import java.io.PrintWriter;

public class DecompileAt extends GhidraScript {
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

        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        try {
            for (int index = 1; index < arguments.length; ++index) {
                Address address = toAddr(arguments[index]);
                Function function =
                    currentProgram.getFunctionManager().getFunctionContaining(address);
                if (function == null) {
                    printerr("No function contains " + address);
                    continue;
                }

                DecompileResults results = decompiler.decompileFunction(function, 120, monitor);
                if (!results.decompileCompleted() || results.getDecompiledFunction() == null) {
                    printerr("Unable to decompile " + function + ": " + results.getErrorMessage());
                    continue;
                }

                String filename = function.getEntryPoint() + "_" +
                    function.getName().replaceAll("[^A-Za-z0-9_.-]", "_") + ".c";
                try (PrintWriter writer =
                         new PrintWriter(new File(output, filename), "UTF-8")) {
                    writer.println("/* Address: " + function.getEntryPoint() + " */");
                    writer.println("/* Signature: " + function.getSignature() + " */");
                    writer.println(results.getDecompiledFunction().getC());
                }
                println("Decompiled " + function + " to " + filename);
            }
        }
        finally {
            decompiler.dispose();
        }
    }
}

