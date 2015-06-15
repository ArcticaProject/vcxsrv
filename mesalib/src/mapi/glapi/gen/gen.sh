./glX_server_table.py -f gl_and_glX_API.xml > indirect_table.c
./glX_proto_size.py -m size_h --only-set --header-tag _INDIRECT_SIZE_H_ > indirect_size.h
./glX_proto_size.py -m size_c --only-set > indirect_size.c
./glX_proto_size.py -m size_h --only-get --header-tag _INDIRECT_SIZE_GET_H_ > indirect_size_get.h
./glX_proto_size.py -m size_c --only-get > indirect_size_get.c
./glX_proto_size.py -m reqsize_c > indirect_reqsize.c
./glX_proto_size.py -m reqsize_h --only-get --header-tag _INDIRECT_SIZE_GET_H_ > indirect_reqsize.h
./glX_proto_recv.py -m dispatch_c > indirect_dispatch.c
./glX_proto_recv.py -m dispatch_c -s > indirect_dispatch_swap.c
./glX_proto_recv.py -m dispatch_h -f gl_and_glX_API.xml -s > indirect_dispatch.h
./gl_table.py -f gl_and_es_API.xml > glapitable.h
./gl_gentable.py -f gl_and_es_API.xml > glapi_gentable.c
./gl_table.py -f gl_and_es_API.xml -m remap_table > dispatch.h
./gl_functions.py -f gl_and_es_API.xml > glfunctions.h
# ./gl_offsets.py > glapioffsets.h
./gl_apitemp.py -f gl_and_es_API.xml > glapitemp.h
./gl_procs.py -f gl_and_es_API.xml > glprocs.h

./glX_proto_send.py -m proto > indirect.c
./glX_proto_send.py -m init_h > indirect.h
./glX_proto_send.py -m init_c > indirect_init.c

./gl_enums.py -f gl_and_es_API.xml > enums.c
./remap_helper.py -f gl_and_es_API.xml > remap_helper.h
cp ../../mapi_abi.py .
./mapi_abi.py --printer glapi --mode lib gl_and_es_API.xml > glapi_mapi_tmp.h

