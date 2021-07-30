add_library(Taxy STATIC
        Expression.h Expression.cpp Node.h Declaration.h Statement.cpp Statement.h Declaration.cpp Initializer.cpp Initializer.h Node.cpp NodeVisitor.h)

set_target_properties(Taxy PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Taxy PUBLIC ../parser/nodes)