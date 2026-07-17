// Export vftable symbols whose qualified names contain a requested fragment.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;

import java.io.File;
import java.io.PrintWriter;

public class ListVtables extends GhidraScript {
    private static String csv(String value) {
        return "\"" + value.replace("\"", "\"\"") + "\"";
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length != 2) {
            throw new IllegalArgumentException("Expected an output CSV and name fragment");
        }

        File output = new File(arguments[0]);
        File parent = output.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            throw new IllegalStateException("Unable to create " + parent);
        }

        int matchCount = 0;
        try (PrintWriter writer = new PrintWriter(output, "UTF-8")) {
            writer.println("address,qualifiedName");
            SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols("vftable");
            while (symbols.hasNext() && !monitor.isCancelled()) {
                Symbol candidate = symbols.next();
                String qualifiedName = candidate.getName(true);
                if (!qualifiedName.contains(arguments[1])) {
                    continue;
                }
                writer.println(csv(candidate.getAddress().toString()) + "," +
                    csv(qualifiedName));
                ++matchCount;
            }
        }
        println("Exported " + matchCount + " matching vftables to " + output);
    }
}
