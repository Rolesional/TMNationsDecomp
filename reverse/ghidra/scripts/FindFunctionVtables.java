// Map exact function addresses to the MSVC vftable slots that reference them.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;

import java.io.File;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;

public class FindFunctionVtables extends GhidraScript {
    private static String csv(String value) {
        return "\"" + value.replace("\"", "\"\"") + "\"";
    }

    @Override
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length < 2) {
            throw new IllegalArgumentException(
                "Expected an output CSV and at least one function address");
        }

        File output = new File(arguments[0]);
        File parent = output.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            throw new IllegalStateException("Unable to create " + parent);
        }

        Set<Address> targets = new HashSet<>();
        for (int index = 1; index < arguments.length; ++index) {
            targets.add(toAddr(arguments[index]));
        }

        int matchCount = 0;
        try (PrintWriter writer = new PrintWriter(output, "UTF-8")) {
            writer.println(
                "targetAddress,vftableAddress,qualifiedName,slotIndex,slotOffset,functionName");
            SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols("vftable");
            while (symbols.hasNext() && !monitor.isCancelled()) {
                Symbol table = symbols.next();
                Address tableAddress = table.getAddress();
                for (int slotIndex = 0; slotIndex < 256; ++slotIndex) {
                    Address slotAddress = tableAddress.add(slotIndex * 4L);
                    if (!currentProgram.getMemory().contains(slotAddress)) {
                        break;
                    }
                    long pointer = Integer.toUnsignedLong(
                        currentProgram.getMemory().getInt(slotAddress));
                    Address target = toAddr(pointer);
                    Function function = currentProgram.getFunctionManager().getFunctionAt(target);
                    if (function == null) {
                        break;
                    }
                    if (!targets.contains(target)) {
                        continue;
                    }
                    writer.println(csv(target.toString()) + "," +
                        csv(tableAddress.toString()) + "," +
                        csv(table.getName(true)) + "," + slotIndex + "," +
                        csv(String.format("0x%X", slotIndex * 4)) + "," +
                        csv(function.getName(true)));
                    ++matchCount;
                }
            }
        }
        println("Mapped " + matchCount + " vftable slots to " + output);
    }
}
