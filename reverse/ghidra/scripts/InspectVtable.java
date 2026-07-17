// Export one MSVC vftable and its function entries.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;

import java.io.File;
import java.io.PrintWriter;

public class InspectVtable extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length < 2) {
            throw new IllegalArgumentException(
                "Expected an output file and a class-name fragment");
        }

        int entryCount = arguments.length >= 3 ? Integer.parseInt(arguments[2]) : 32;
        String expectedName = arguments[1] + "::vftable";
        Symbol exactMatch = null;
        Symbol partialMatch = null;
        int partialMatchCount = 0;
        SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols("vftable");
        while (symbols.hasNext()) {
            Symbol candidate = symbols.next();
            String qualifiedName = candidate.getName(true);
            if (qualifiedName.equals(expectedName)) {
                exactMatch = candidate;
                break;
            }
            if (qualifiedName.contains(arguments[1])) {
                partialMatch = candidate;
                partialMatchCount++;
            }
        }
        Symbol match = exactMatch;
        if (match == null && partialMatchCount == 1) {
            match = partialMatch;
        }
        if (match == null) {
            if (partialMatchCount > 1) {
                throw new IllegalArgumentException(
                    "No exact vftable named " + expectedName + "; " + partialMatchCount +
                    " partial matches are ambiguous");
            }
            throw new IllegalArgumentException("No vftable found for " + arguments[1]);
        }

        File output = new File(arguments[0]);
        File parent = output.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            throw new IllegalStateException("Unable to create " + parent);
        }

        try (PrintWriter writer = new PrintWriter(output, "UTF-8")) {
            Address tableAddress = match.getAddress();
            writer.println("vftable=" + match.getName(true));
            writer.println("address=" + tableAddress);
            int emittedCount = 0;
            for (int index = 0; index < entryCount; ++index) {
                Address slot = tableAddress.add(index * 4L);
                long pointer = Integer.toUnsignedLong(currentProgram.getMemory().getInt(slot));
                Address target = toAddr(pointer);
                Function function = currentProgram.getFunctionManager().getFunctionAt(target);
                if (function == null) {
                    break;
                }
                writer.printf("%02d +0x%02X %s %s%n", index, index * 4, target,
                    function.getName(true));
                ++emittedCount;
            }
            writer.println("entries=" + emittedCount);
        }
        println("Exported " + match.getName(true) + " to " + output);
    }
}
