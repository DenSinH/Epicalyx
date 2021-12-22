add_library(Optimizer STATIC
        ProgramDependencies.h
        ProgramDependencies.cpp BasicOptimizer.cpp BasicOptimizer.h RemoveUnused.cpp RemoveUnused.h)