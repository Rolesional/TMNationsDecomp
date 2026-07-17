// Exports stable, diffable recovery metadata after Ghidra auto-analysis.
// @category TMNF

import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.DataType;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.ExternalLocation;
import ghidra.program.model.symbol.ExternalLocationIterator;
import ghidra.program.model.symbol.ExternalManager;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;

import java.io.File;
import java.io.PrintWriter;

public class ExportProgramInventory extends GhidraScript {
    private static String csv(String value) {
        if (value == null) {
            return "\"\"";
        }
        return "\"" + value.replace("\"", "\"\"").replace("\r", "\\r").replace("\n", "\\n") + "\"";
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

        int functionCount = 0;
        try (PrintWriter writer = new PrintWriter(new File(output, "functions.csv"), "UTF-8")) {
            writer.println("address,name,size,signature");
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext() && !monitor.isCancelled()) {
                Function function = functions.next();
                writer.println(
                    csv(function.getEntryPoint().toString()) + "," +
                    csv(function.getName(true)) + "," +
                    function.getBody().getNumAddresses() + "," +
                    csv(function.getSignature().getPrototypeString()));
                functionCount++;
            }
        }

        int stringCount = 0;
        int classStringCount = 0;
        int classStringReferenceCount = 0;
        ReferenceManager referenceManager = currentProgram.getReferenceManager();
        PrintWriter classWriter = new PrintWriter(new File(output, "class-strings.csv"), "UTF-8");
        PrintWriter classReferenceWriter =
            new PrintWriter(new File(output, "class-string-references.csv"), "UTF-8");
        classWriter.println("address,value");
        classReferenceWriter.println("stringAddress,value,fromAddress,function,referenceType");
        try (PrintWriter writer = new PrintWriter(new File(output, "defined-strings.csv"), "UTF-8")) {
            writer.println("address,type,value");
            DataIterator dataIterator = currentProgram.getListing().getDefinedData(true);
            while (dataIterator.hasNext() && !monitor.isCancelled()) {
                Data data = dataIterator.next();
                DataType type = data.getDataType();
                Object value = data.getValue();
                if (value instanceof String) {
                    String stringValue = (String)value;
                    writer.println(
                        csv(data.getAddress().toString()) + "," + csv(type.getName()) + "," +
                        csv(stringValue));
                    stringCount++;
                    if (stringValue.matches("^(?:C[A-Z][A-Za-z0-9_:<>~]*|GbxGame(?:::[A-Za-z0-9_:<>~]+)?)$")) {
                        classWriter.println(
                            csv(data.getAddress().toString()) + "," + csv(stringValue));
                        classStringCount++;

                        ReferenceIterator references =
                            referenceManager.getReferencesTo(data.getAddress());
                        while (references.hasNext() && !monitor.isCancelled()) {
                            Reference reference = references.next();
                            Function function = currentProgram.getFunctionManager()
                                .getFunctionContaining(reference.getFromAddress());
                            classReferenceWriter.println(
                                csv(data.getAddress().toString()) + "," + csv(stringValue) + "," +
                                csv(reference.getFromAddress().toString()) + "," +
                                csv(function == null ? "" : function.getName(true)) + "," +
                                csv(reference.getReferenceType().toString()));
                            classStringReferenceCount++;
                        }
                    }
                }
            }
        }
        classWriter.close();
        classReferenceWriter.close();

        int importCount = 0;
        try (PrintWriter writer = new PrintWriter(new File(output, "imports.csv"), "UTF-8")) {
            writer.println("library,label,address");
            ExternalManager externalManager = currentProgram.getExternalManager();
            for (String libraryName : externalManager.getExternalLibraryNames()) {
                ExternalLocationIterator locations = externalManager.getExternalLocations(libraryName);
                while (locations.hasNext() && !monitor.isCancelled()) {
                    ExternalLocation location = locations.next();
                    writer.println(
                        csv(location.getLibraryName()) + "," +
                        csv(location.getLabel()) + "," +
                        csv(location.getAddress() == null ? "" : location.getAddress().toString()));
                    importCount++;
                }
            }
        }

        try (PrintWriter writer = new PrintWriter(new File(output, "summary.txt"), "UTF-8")) {
            writer.println("program=" + currentProgram.getName());
            writer.println("language=" + currentProgram.getLanguageID());
            writer.println("compiler=" + currentProgram.getCompilerSpec().getCompilerSpecID());
            writer.println("imageBase=" + currentProgram.getImageBase());
            writer.println("functions=" + functionCount);
            writer.println("definedStrings=" + stringCount);
            writer.println("classStrings=" + classStringCount);
            writer.println("classStringReferences=" + classStringReferenceCount);
            writer.println("imports=" + importCount);
        }

        println("Exported " + functionCount + " functions, " + stringCount + " strings, and " + importCount + " imports to " + output);
    }
}
