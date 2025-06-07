add_executable (tcc
  tcc.cpp
   )
target_link_libraries(tcc PRIVATE ${llvm_libs})
