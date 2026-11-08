// Ghidra headless script: export decompiled C for every non-library function.
// Usage: analyzeHeadless <proj> <name> -import <bin> -postScript ExportDecomp.java <outdir>
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import java.io.File;
import java.io.PrintWriter;
import java.util.Set;
import java.util.TreeSet;

public class ExportDecomp extends GhidraScript {
    private static final String[] SKIP = {
        "std::", "__gnu", "Eigen::", "__cxx", "_GLOBAL_", "operator new", "operator delete",
        "__static_initialization", "_init", "_fini", "frame_dummy", "register_tm_clones",
        "deregister_tm_clones", "__do_global", "progschj::", "cub::", "thrust::", "__cudart", "cudart::", "__libc", "_ITM_", "__gmon",
    };

    private boolean skip(Function f, String name) {
        if (f.isThunk() || f.isExternal()) return true;
        for (String s : SKIP) if (name.contains(s)) return true;
        return false;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File out = new File(args[0]);
        File fdir = new File(out, "functions");
        fdir.mkdirs();
        DecompInterface ifc = new DecompInterface();
        DecompileOptions opt = new DecompileOptions();
        ifc.setOptions(opt);
        ifc.toggleCCode(true);
        ifc.toggleSyntaxTree(false);
        ifc.openProgram(currentProgram);
        PrintWriter index = new PrintWriter(new File(out, "index.tsv"), "UTF-8");
        index.println("address\tsize\tname\tfile\tcallees");
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        int n = 0, failed = 0;
        while (it.hasNext() && !monitor.isCancelled()) {
            Function f = it.next();
            String name = f.getName(true);
            if (skip(f, name)) continue;
            String file = String.format("%s_%s.c", f.getEntryPoint().toString(),
                    name.replaceAll("[^A-Za-z0-9_]+", "_"));
            if (file.length() > 150) file = file.substring(0, 150) + ".c";
            DecompileResults r = ifc.decompileFunction(f, 300, monitor);
            Set<String> callees = new TreeSet<>();
            for (Function c : f.getCalledFunctions(monitor)) callees.add(c.getName(true));
            try (PrintWriter w = new PrintWriter(new File(fdir, file), "UTF-8")) {
                w.println("// " + name);
                w.println("// signature: " + f.getSignature().getPrototypeString());
                w.println("// entry " + f.getEntryPoint() + " size " + f.getBody().getNumAddresses());
                w.println("// callees: " + String.join(", ", callees));
                if (r != null && r.decompileCompleted()) {
                    w.println(r.getDecompiledFunction().getC());
                } else {
                    failed++;
                    w.println("// DECOMPILE FAILED: " + (r == null ? "null" : r.getErrorMessage()));
                }
            }
            index.printf("%s\t%d\t%s\t%s\t%s%n", f.getEntryPoint(), f.getBody().getNumAddresses(),
                    name, file, String.join("|", callees));
            n++;
        }
        index.close();
        ifc.dispose();
        println("EXPORTED " + n + " functions, failed " + failed);
    }
}
