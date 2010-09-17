echo on
glx_server_table.py -f gl_and_glX_API.xml > indirect_table.c
glx_proto_size.py -f gl_and_glX_API.xml -m size_h --only-set > indirect_size.h
glx_proto_size.py -f gl_and_glX_API.xml -m size_h --only-get > indirect_size_get.h
glx_proto_size.py -f gl_and_glX_API.xml -m size_c --only-get > indirect_size_get.c
glx_proto_size.py -f gl_and_glX_API.xml -m reqsize_c > indirect_reqsize.c
glx_proto_size.py -f gl_and_glX_API.xml -m reqsize_h > indirect_reqsize.h
glx_proto_recv.py -f gl_and_glX_API.xml > indirect_dispatch.c
glx_proto_recv.py -f gl_and_glX_API.xml -s > indirect_dispatch_swap.c
glx_proto_recv.py -f gl_and_glX_API.xml -m dispatch_h > indirect_dispatch.h
gl_table.py -f gl_and_glX_API.xml -m table > glapitable.h
gl_table.py -f gl_and_glX_API.xml -m remap_table > dispatch.h
gl_offsets.py -f gl_and_glX_API.xml > glapioffsets.h
gl_apitemp.py -f gl_and_glX_API.xml > glapitemp.h
gl_procs.py -f gl_and_glX_API.xml > glprocs.h
