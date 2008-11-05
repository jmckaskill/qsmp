project(tcl)

set(tcl_INCLUDE_DIR ${tcl_INCLUDE_DIR} CACHE INTERNAL "tcl include dir")

set(includes tcl/associative_tree.h
             tcl/associative_tree.inl
             tcl/basic_tree.h
             tcl/basic_tree.inl
             tcl/child_iterator.h
             tcl/child_node_iterator.h
             tcl/descendant_iterator.h
             tcl/descendant_iterator.inl
             tcl/descendant_node_iterator.h
             tcl/descendant_node_iterator.inl
             tcl/multitree.h
             tcl/multitree.inl
             tcl/ordered_iterator.h
             tcl/reverse_iterator.h
             tcl/reverse_node_iterator.h
             tcl/sequential_tree.h
             tcl/sequential_tree.inl
             tcl/tree.h
             tcl/tree.inl
             tcl/unique_tree.h
             tcl/unique_tree.inl
             dummy.cpp)

add_library(tcl STATIC ${includes})
