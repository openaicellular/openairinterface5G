####################################################################################
######                           nr_ulsim unit test                           ######
####################################################################################
set(default_ldpc "")
set(slot_demo "--loader.ldpc.shlibversion" "_slot_demo")
set(t2_offload_nr_ulsim "-o" "-O" "${CMAKE_CURRENT_SOURCE_DIR}/t2_offload.conf")
set(slot_t2 "-O" "${CMAKE_CURRENT_SOURCE_DIR}/slot_t2.conf")
set(slot_xdma "-O" "${CMAKE_CURRENT_SOURCE_DIR}/slot_xdma.conf")

set(NR_ULSIM_OPTION_TO_TEST_LIST
  default_ldpc
  slot_demo
  slot_xdma
)

if (ENABLE_LDPC_T2)
  list(APPEND NR_ULSIM_OPTION_TO_TEST_LIST
    t2_offload_nr_ulsim
    #slot_t2
  )
endif()

set(test_nr_ulsim_misc_test1 "-n100" "-m9" "-r106" "-s5")
set(test_nr_ulsim_misc_test2 "-n100" "-m16" "-s10")
set(test_nr_ulsim_misc_test3 "-n100" "-m28" "-s20")
set(test_nr_ulsim_misc_test4 "-n100" "-m27" "-s25" "-q1")
set(test_nr_ulsim_misc_test5 "-n100" "-m9" "-R217" "-r217" "-s5")
set(test_nr_ulsim_misc_test6 "-n100" "-m9" "-R273" "-r273" "-s5")
set(test_nr_ulsim_misc_test7 "-n100" "-s5" "-U0,1,1,1")
set(test_nr_ulsim_misc_test8 "-n100" "-s5" "-T1,2" "-U0,2,1,1")
set(test_nr_ulsim_misc_test9 "-n100" "-s5" "-T2,2" "-U1,2,1,1")
set(test_nr_ulsim_misc_test10 "-n100" "-s5" "-a4" "-b8" "-T1,2" "-U1,3,1,1")
set(test_nr_ulsim_misc_test11 "-n100" "-u0" "-m0" "-R25" "-r25" "-i1,0")
set(test_nr_ulsim_misc_test12 "-n100" "-m0" "-S" "-0.6" "-i1,0")
set(test_nr_ulsim_misc_test13 "-n100" "-m28" "-R106" "-r106" "-t90" "-s24" "-S24" "-d8")
set(test_nr_ulsim_sc_test1 "-n100" "-s5" "-Z")
set(test_nr_ulsim_sc_test2 "-n100" "-s5" "-Z" "-r75")
set(test_nr_ulsim_sc_test3 "-n50" "-s5" "-Z" "-r216" "-R217")
set(test_nr_ulsim_sc_test4 "-n50" "-s5" "-Z" "-r270" "-R273")
set(test_nr_ulsim_sc_test5 "-n100" "-s5" "-Z" "-U0,2,1,2")
set(test_nr_ulsim_mimo_test1 "-n100" "-m19" "-s10" "-S15" "-z2")
set(test_nr_ulsim_mimo_test2 "-n100" "-m9" "-r106" "-s8" "-W2" "-y2" "-z2")
set(test_nr_ulsim_mimo_test3 "-n100" "-m10" "-r106" "-s12" "-W2" "-y2" "-z2")
set(test_nr_ulsim_mimo_test4 "-n100" "-m19" "-r106" "-s22" "-W2" "-y2" "-z2")
set(test_nr_ulsim_mimo_test5 "-n100" "-m9" "-r106" "-s10" "-W4" "-y4" "-z4")
set(test_nr_ulsim_3gpp_test1 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R106" "-r106" "-U0,1,1,2" "-z2" "-s12.4" "-S12.4")
set(test_nr_ulsim_3gpp_test2 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R106" "-r106" "-U0,1,1,2" "-z4" "-s8.5" "-S8.5")
set(test_nr_ulsim_3gpp_test3 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R106" "-r106" "-U0,1,1,2" "-z8" "-s5.4" "-S5.4")
set(test_nr_ulsim_3gpp_test4 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R25" "-r25" "-U1,1,1,2" "-z2" "-s12.5" "-S12.5")
set(test_nr_ulsim_3gpp_test5 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R25" "-r25" "-U1,1,1,2" "-z4" "-s8.9" "-S8.9")
set(test_nr_ulsim_3gpp_test6 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R25" "-r25" "-U1,1,1,2" "-z8" "-s5.7" "-S5.7")
set(test_nr_ulsim_3gpp_test7 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R52" "-r52" "-U1,1,1,2" "-z2" "-s12.6" "-S12.6")
set(test_nr_ulsim_3gpp_test8 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R52" "-r52" "-U1,1,1,2" "-z4" "-s8.9" "-S8.9")
set(test_nr_ulsim_3gpp_test9 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R52" "-r52" "-U1,1,1,2" "-z8" "-s5.8" "-S5.8")
set(test_nr_ulsim_3gpp_test10 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R106" "-r106" "-U1,1,1,2" "-z2" "-s12.3" "-S12.3")
set(test_nr_ulsim_3gpp_test11 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R106" "-r106" "-U1,1,1,2" "-z4" "-s8.8" "-S8.8")
set(test_nr_ulsim_3gpp_test12 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u0" "-m20" "-R106" "-r106" "-U1,1,1,2" "-z8" "-s5.7" "-S5.7")
set(test_nr_ulsim_3gpp_test13 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R24" "-r24" "-U1,1,1,2" "-z2" "-s12.5" "-S12.5")
set(test_nr_ulsim_3gpp_test14 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R24" "-r24" "-U1,1,1,2" "-z4" "-s8.6" "-S8.6")
set(test_nr_ulsim_3gpp_test15 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R24" "-r24" "-U1,1,1,2" "-z8" "-s5.6" "-S5.6")
set(test_nr_ulsim_3gpp_test16 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R51" "-r51" "-U1,1,1,2" "-z2" "-s12.5" "-S12.5")
set(test_nr_ulsim_3gpp_test17 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R51" "-r51" "-U1,1,1,2" "-z4" "-s8.6" "-S8.6")
set(test_nr_ulsim_3gpp_test18 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R51" "-r51" "-U1,1,1,2" "-z8" "-s5.6" "-S5.6")
set(test_nr_ulsim_3gpp_test19 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R106" "-r106" "-U1,1,1,2" "-z2" "-s12.5" "-S12.5")
set(test_nr_ulsim_3gpp_test20 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R106" "-r106" "-U1,1,1,2" "-z4" "-s8.7" "-S8.7")
set(test_nr_ulsim_3gpp_test21 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R106" "-r106" "-U1,1,1,2" "-z8" "-s5.5" "-S5.5")
set(test_nr_ulsim_3gpp_test22 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R273" "-r273" "-U1,1,1,2" "-z2" "-s13.1" "-S13.1")
set(test_nr_ulsim_3gpp_test23 "-n100" "-b14" "-I7" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R273" "-r273" "-U1,1,1,2" "-z4" "-s9.2" "-S9.2")
set(test_nr_ulsim_3gpp_test24 "-n100" "-b14" "-I8" "-i0,1" "-gA,l,10" "-t70" "-u1" "-m20" "-R273" "-r273" "-U1,1,1,2" "-z8" "-s5.9" "-S5.9")
set(test_nr_ulsim_3gpp_test25 "-n100" "-b14" "-I15" "-i0,1" "-gB,l" "-t70" "-u1" "-m2" "-R106" "-r106" "-U1,1,1,2" "-W2" "-y2" "-z2" "-s1.7" "-S1.7")
set(test_nr_ulsim_3gpp_test26 "-n100" "-b14" "-I15" "-i0,1" "-gB,l" "-t70" "-u1" "-m2" "-R106" "-r106" "-U1,1,1,2" "-W2" "-y2" "-z4" "-s-2.1" "-S-2.1")
set(test_nr_ulsim_3gpp_test27 "-n100" "-b14" "-I15" "-i0,1" "-gC,l" "-t70" "-u1" "-m16" "-R106" "-r106" "-U1,1,1,2" "-W2" "-y2" "-z2" "-s18.7" "-S18.7")
set(test_nr_ulsim_3gpp_test28 "-n100" "-b14" "-I15" "-i0,1" "-gC,l" "-t70" "-u1" "-m16" "-R106" "-r106" "-U1,1,1,2" "-W2" "-y2" "-z4" "-s11.2" "-S11.2")

set(NR_ULSIM_TEST_CASE_OPTION_LIST
  test_nr_ulsim_misc_test1
  test_nr_ulsim_misc_test2
  test_nr_ulsim_misc_test3
  test_nr_ulsim_misc_test4
  test_nr_ulsim_misc_test5
  test_nr_ulsim_misc_test6
  test_nr_ulsim_misc_test7
  test_nr_ulsim_misc_test8
  test_nr_ulsim_misc_test9
  test_nr_ulsim_misc_test10
  test_nr_ulsim_misc_test11
  test_nr_ulsim_misc_test12
  test_nr_ulsim_misc_test13
  test_nr_ulsim_sc_test1
  test_nr_ulsim_sc_test2
  test_nr_ulsim_sc_test3
  test_nr_ulsim_sc_test4
  test_nr_ulsim_sc_test5
  test_nr_ulsim_mimo_test1
  test_nr_ulsim_mimo_test2
  test_nr_ulsim_mimo_test3
  test_nr_ulsim_mimo_test4
  test_nr_ulsim_mimo_test5
  test_nr_ulsim_3gpp_test1
  test_nr_ulsim_3gpp_test2
  test_nr_ulsim_3gpp_test3
  test_nr_ulsim_3gpp_test4
  test_nr_ulsim_3gpp_test5
  test_nr_ulsim_3gpp_test6
  test_nr_ulsim_3gpp_test7
  test_nr_ulsim_3gpp_test8
  test_nr_ulsim_3gpp_test9
  test_nr_ulsim_3gpp_test10
  test_nr_ulsim_3gpp_test11
  test_nr_ulsim_3gpp_test12
  test_nr_ulsim_3gpp_test13
  test_nr_ulsim_3gpp_test14
  test_nr_ulsim_3gpp_test15
  test_nr_ulsim_3gpp_test16
  test_nr_ulsim_3gpp_test17
  test_nr_ulsim_3gpp_test18
  test_nr_ulsim_3gpp_test19
  test_nr_ulsim_3gpp_test20
  test_nr_ulsim_3gpp_test21
  test_nr_ulsim_3gpp_test22
  test_nr_ulsim_3gpp_test23
  test_nr_ulsim_3gpp_test24
  test_nr_ulsim_3gpp_test25
  test_nr_ulsim_3gpp_test26
  test_nr_ulsim_3gpp_test27
  test_nr_ulsim_3gpp_test28
)

foreach(NR_ULSIM_OPTION_TO_TEST IN LISTS NR_ULSIM_OPTION_TO_TEST_LIST)
  foreach(NR_ULSIM_TEST_CASE IN LISTS NR_ULSIM_TEST_CASE_OPTION_LIST)
    add_test(
      NAME ${NR_ULSIM_TEST_CASE}_${NR_ULSIM_OPTION_TO_TEST}
      COMMAND nr_ulsim -P -C1 ${${NR_ULSIM_TEST_CASE}} ${${NR_ULSIM_OPTION_TO_TEST}}
      COMMAND_EXPAND_LISTS
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  endforeach()
endforeach()

####################################################################################
######                         nr_ulschsim unit test                          ######
####################################################################################

set(NR_ULSCHSIM_OPTION_TO_TEST_LIST
  default_ldpc
  slot_demo
)

#if (ENABLE_LDPC_T2)
#  list(APPEND NR_ULSCHSIM_OPTION_TO_TEST_LIST
#    slot_t2
#  )
#endif()

set(test_nr_ulschsim_test1 "-R" "106" "-m9" "-s13" "-n100")
set(test_nr_ulschsim_test2 "-R" "217" "-m15" "-s15" "-n100")
set(test_nr_ulschsim_test3 "-R" "273" "-m19" "-s20" "-n100")
set(test_nr_ulschsim_test4 "-R" "106" "-m9" "-s13" "-n100" "-y4" "-z4" "-W4")

set(NR_ULSCHSIM_TEST_CASE_OPTION_LIST
  test_nr_ulschsim_test1
  test_nr_ulschsim_test2
  test_nr_ulschsim_test3
  test_nr_ulschsim_test4
)

foreach(NR_ULSCHSIM_OPTION_TO_TEST IN LISTS NR_ULSCHSIM_OPTION_TO_TEST_LIST)
  foreach(NR_ULSCHSIM_TEST_CASE IN LISTS NR_ULSCHSIM_TEST_CASE_OPTION_LIST)
    add_test(
      NAME ${NR_ULSCHSIM_TEST_CASE}_${NR_ULSCHSIM_OPTION_TO_TEST}
      COMMAND nr_ulschsim ${${NR_ULSCHSIM_TEST_CASE}} ${${NR_ULSCHSIM_OPTION_TO_TEST}}
      COMMAND_EXPAND_LISTS
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  endforeach()
endforeach()

####################################################################################
######                           nr_dlsim unit test                           ######
####################################################################################
set(t2_offload_nr_dlsim "-c" "-O" "${CMAKE_CURRENT_SOURCE_DIR}/t2_offload.conf")

set(NR_DLSIM_OPTION_TO_TEST_LIST
  default_ldpc
  slot_demo
)

if (ENABLE_LDPC_T2)
  list(APPEND NR_DLSIM_OPTION_TO_TEST_LIST
    t2_offload_nr_dlsim
    #slot_t2
  )
endif()

set(test_nr_dlsim_basic_test1 "-n100" "-R106" "-b106" "-s5")
set(test_nr_dlsim_basic_test2 "-n100" "-R217" "-b217" "-s5")
set(test_nr_dlsim_basic_test3 "-n100" "-R273" "-b273" "-s5")
set(test_nr_dlsim_basic_test4 "-n100" "-s1" "-S2" "-t25")
set(test_nr_dlsim_basic_test5 "-n100" "-s1" "-S2" "-t33")
set(test_nr_dlsim_basic_test6 "-n100" "-s5" "-S7" "-t50")
set(test_nr_dlsim_basic_test7 "-n100" "-m0" "-e0" "-R25" "-b25" "-i" "2" "1" "0")
set(test_nr_dlsim_offset_test1 "-n100" "-R106" "-a25" "-s5")
set(test_nr_dlsim_offset_test2 "-n100" "-R106" "-a51" "-s5")
set(test_nr_dlsim_offset_test3 "-n100" "-R217" "-b100" "-s5")
set(test_nr_dlsim_offset_test4 "-n100" "-R217" "-a80" "-s5")
set(test_nr_dlsim_offset_test5 "-n100" "-R217" "-a110" "-s5" "-b100")
set(test_nr_dlsim_mcs_mimo_test1 "-n100" "-e27" "-s30")
set(test_nr_dlsim_mcs_mimo_test2 "-n100" "-e16" "-s11" "-S13")
set(test_nr_dlsim_mcs_mimo_test3 "-n100" "-q1" "-e26" "-s30")
set(test_nr_dlsim_mcs_mimo_test4 "-n100" "-e0 -t95" "-S-1.0" "-i" "2" "1" "0")
set(test_nr_dlsim_mcs_mimo_test5 "-n10" "-s20" "-U" "3" "0" "0" "2" "-gA" "-x1" "-y4" "-z4")
set(test_nr_dlsim_mcs_mimo_test6 "-n10" "-s20" "-U" "3" "0" "0" "2" "-gA" "-x2" "-y4" "-z4")
set(test_nr_dlsim_mcs_mimo_test7 "-n10" "-s20" "-U" "3" "0" "0" "2" "-x4" "-y4" "-z4")
set(test_nr_dlsim_dmrs_ptrs_test1 "-n100" "-s5" "-T" "2" "2" "2")
set(test_nr_dlsim_dmrs_ptrs_test2 "-n100" "-s5" "-T" "2" "1" "2")
set(test_nr_dlsim_dmrs_ptrs_test3 "-n100" "-s5" "-T" "2" "0" "4")
set(test_nr_dlsim_dmrs_ptrs_test4 "-n100" "-s5" "-S7" "-U" "2" "0" "1")
set(test_nr_dlsim_dmrs_ptrs_test5 "-n100" "-s5" "-S7" "-U" "2" "0" "2")
set(test_nr_dlsim_dmrs_ptrs_test6 "-n100" "-s5" "-S7" "-U" "2" "1" "3")

set(NR_DLSIM_TEST_CASE_OPTION_LIST
  test_nr_dlsim_basic_test1
  test_nr_dlsim_basic_test2
  test_nr_dlsim_basic_test3
  test_nr_dlsim_basic_test4
  test_nr_dlsim_basic_test5
  test_nr_dlsim_basic_test6
  test_nr_dlsim_basic_test7
  test_nr_dlsim_offset_test1
  test_nr_dlsim_offset_test2
  test_nr_dlsim_offset_test3
  test_nr_dlsim_offset_test4
  test_nr_dlsim_offset_test5
  test_nr_dlsim_mcs_mimo_test1
  test_nr_dlsim_mcs_mimo_test2
  test_nr_dlsim_mcs_mimo_test3
  test_nr_dlsim_mcs_mimo_test4
  test_nr_dlsim_mcs_mimo_test5
  test_nr_dlsim_mcs_mimo_test6
  test_nr_dlsim_mcs_mimo_test7
  test_nr_dlsim_dmrs_ptrs_test1
  test_nr_dlsim_dmrs_ptrs_test2
  test_nr_dlsim_dmrs_ptrs_test3
  test_nr_dlsim_dmrs_ptrs_test4
  test_nr_dlsim_dmrs_ptrs_test5
  test_nr_dlsim_dmrs_ptrs_test6
)

foreach(NR_DLSIM_OPTION_TO_TEST IN LISTS NR_DLSIM_OPTION_TO_TEST_LIST)
  foreach(NR_DLSIM_TEST_CASE IN LISTS NR_DLSIM_TEST_CASE_OPTION_LIST)
    add_test(
      NAME ${NR_DLSIM_TEST_CASE}_${NR_DLSIM_OPTION_TO_TEST}
      COMMAND nr_dlsim -P ${${NR_DLSIM_TEST_CASE}} ${${NR_DLSIM_OPTION_TO_TEST}}
      COMMAND_EXPAND_LISTS
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    set_property(
      TEST ${NR_DLSIM_TEST_CASE}_${NR_DLSIM_OPTION_TO_TEST}
      PROPERTY PASS_REGULAR_EXPRESSION "PDSCH test OK"
    )
  endforeach()
endforeach()

####################################################################################
######                         nr_dlschsim unit test                          ######
####################################################################################

set(NR_DLSCHSIM_OPTION_TO_TEST_LIST
  default_ldpc
  slot_demo
)

#if (ENABLE_LDPC_T2)
#  list(APPEND NR_DLSCHSIM_OPTION_TO_TEST_LIST
#    slot_t2
#  )
#endif()

set(test_nr_dlschsim_test1 "-R" "106" "-m9" "-s13" "-n100")
set(test_nr_dlschsim_test2 "-R" "217" "-m15" "-s15" "-n100")
set(test_nr_dlschsim_test3 "-R" "273" "-m19" "-s20" "-n100")

set(NR_DLSCHSIM_TEST_CASE_OPTION_LIST
  test_nr_dlschsim_test1
  test_nr_dlschsim_test2
  test_nr_dlschsim_test3
)

foreach(NR_DLSCHSIM_OPTION_TO_TEST IN LISTS NR_DLSCHSIM_OPTION_TO_TEST_LIST)
  foreach(NR_DLSCHSIM_TEST_CASE IN LISTS NR_DLSCHSIM_TEST_CASE_OPTION_LIST)
    add_test(
      NAME ${NR_DLSCHSIM_TEST_CASE}_${NR_DLSCHSIM_OPTION_TO_TEST}
      COMMAND nr_dlschsim ${${NR_DLSCHSIM_TEST_CASE}} ${${NR_DLSCHSIM_OPTION_TO_TEST}}
      COMMAND_EXPAND_LISTS
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  endforeach()
endforeach()

####################################################################################
######                         nr-softmodem unit test                         ######
####################################################################################
set(default_ldpc "-O" "${CMAKE_CURRENT_SOURCE_DIR}/gnb.sa.band78.106prb.rfsim.2x2.conf")
set(slot_demo "-O" "${CMAKE_CURRENT_SOURCE_DIR}/gnb.sa.band78.106prb.rfsim.2x2.conf" "--loader.ldpc.shlibversion" "_slot_demo")
set(t2_offload "-O" "${CMAKE_CURRENT_SOURCE_DIR}/gnb.sa.band78.106prb.rfsim.2x2.t2_offload.conf" "--ldpc-offload-enable")
set(slot_t2 "-O" "${CMAKE_CURRENT_SOURCE_DIR}/gnb.sa.band78.106prb.rfsim.2x2.slot_t2.conf")
set(slot_xdma "-O" "${CMAKE_CURRENT_SOURCE_DIR}/gnb.sa.band78.106prb.rfsim.2x2.slot_xdma.conf")

set(NR_SOFTMODEM_OPTION_TO_TEST_LIST
  default_ldpc
  slot_demo
  slot_xdma
)

if (ENABLE_LDPC_T2)
  list(APPEND NR_SOFTMODEM_OPTION_TO_TEST_LIST
    t2_offload
    #slot_t2
  )
endif()

foreach(NR_SOFTMODEM_OPTION_TO_TEST IN LISTS NR_SOFTMODEM_OPTION_TO_TEST_LIST)
  add_test(
    NAME two_ues_rfsim_${NR_SOFTMODEM_OPTION_TO_TEST}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/two_ues_rfsim.sh ${${NR_SOFTMODEM_OPTION_TO_TEST}}
    COMMAND_EXPAND_LISTS
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endforeach()

####################################################################################
######                        nr-uesoftmodem unit test                       ######
####################################################################################
set(default_ldpc "-O" "${CMAKE_CURRENT_SOURCE_DIR}/nrue.uicc.1.conf")
set(slot_demo "-O" "${CMAKE_CURRENT_SOURCE_DIR}/nrue.uicc.1.slot_demo.conf")
set(slot_t2 "-O" "${CMAKE_CURRENT_SOURCE_DIR}/nrue.uicc.1.slot_t2.conf")

set(NR_UESOFTMODEM_OPTION_TO_TEST_LIST
  default_ldpc
  slot_demo
)

#if (ENABLE_LDPC_T2)
#  list(APPEND NR_UESOFTMODEM_OPTION_TO_TEST_LIST
#    slot_t2
#  )
#endif()

foreach(NR_UESOFTMODEM_OPTION_TO_TEST IN LISTS NR_UESOFTMODEM_OPTION_TO_TEST_LIST)
  add_test(
    NAME ue_coding_rfsim_${NR_UESOFTMODEM_OPTION_TO_TEST}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ue_coding_rfsim.sh ${${NR_UESOFTMODEM_OPTION_TO_TEST}}
    COMMAND_EXPAND_LISTS
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endforeach()

