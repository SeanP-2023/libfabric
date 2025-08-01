configdir = $(datarootdir)/${PACKAGE_NAME}
AM_CFLAGS = -g -Wall -D_GNU_SOURCE -D 'CONFIG_PATH="${configdir}"' -I$(srcdir)/include
ACLOCAL_AMFLAGS = -I config

if MACOS
os_excludes = -f ./test_configs/osx.exclude
AM_CFLAGS += -I$(srcdir)/include/osx
endif

if FREEBSD
os_excludes = -f ./test_configs/freebsd.exclude
AM_CFLAGS += -I$(srcdir)/include/freebsd
endif

bin_PROGRAMS = \
	functional/fi_av_xfer \
	functional/fi_msg \
	functional/fi_msg_sockets \
	functional/fi_rdm \
	functional/fi_rdm_rma_event \
	functional/fi_rdm_rma_trigger \
	functional/fi_rdm_deferred_wq \
	functional/fi_dgram \
	functional/fi_mcast \
	functional/fi_rdm_tagged_peek \
	functional/fi_cq_data \
	functional/fi_scalable_ep \
	functional/fi_shared_ctx \
	functional/fi_msg_epoll \
	functional/fi_rdm_shared_av \
	functional/fi_cm_data \
	functional/fi_multi_mr \
	functional/fi_rdm_multi_domain \
	functional/fi_multi_ep \
	functional/fi_recv_cancel \
	functional/fi_unexpected_msg \
	functional/fi_unmap_mem \
	functional/fi_inject_test \
	functional/fi_resmgmt_test \
	functional/fi_rdm_atomic \
	functional/fi_rdm_stress \
	functional/fi_multi_recv \
	functional/fi_flood \
	functional/fi_rdm_multi_client \
	functional/fi_loopback \
	benchmarks/fi_msg_pingpong \
	benchmarks/fi_msg_bw \
	benchmarks/fi_rma_bw \
	benchmarks/fi_rdm_cntr_pingpong \
	benchmarks/fi_dgram_pingpong \
	benchmarks/fi_rdm_pingpong \
	benchmarks/fi_rma_pingpong \
	benchmarks/fi_rdm_tagged_pingpong \
	benchmarks/fi_rdm_bw \
	benchmarks/fi_rdm_bw_mt \
	benchmarks/fi_rdm_tagged_bw \
	benchmarks/fi_rma_tx_completion \
	unit/fi_eq_test \
	unit/fi_cq_test \
	unit/fi_mr_test \
	unit/fi_mr_cache_evict \
	unit/fi_cntr_test \
	unit/fi_av_test \
	unit/fi_dom_test \
	unit/fi_getinfo_test \
	unit/fi_setopt_test \
	ubertest/fi_ubertest	\
	multinode/fi_multinode	\
	multinode/fi_multinode_coll \
	component/sock_test \
	regression/sighandler_test \
	common/check_hmem

if HAVE_ZE_DEVEL
if HAVE_VERBS_DEVEL
bin_PROGRAMS += \
	component/dmabuf-rdma/xe_rdmabw \
	component/dmabuf-rdma/xe_mr_reg
endif HAVE_VERBS_DEVEL
bin_PROGRAMS += \
	component/dmabuf-rdma/fi_xe_rdmabw \
	component/dmabuf-rdma/fi_xe_mr_reg \
	component/dmabuf-rdma/xe_memcopy
endif HAVE_ZE_DEVEL

dist_bin_SCRIPTS = \
	scripts/runfabtests.sh \
	scripts/runfabtests.py \
	scripts/runmultinode.sh \
	scripts/runmultinode.py \
	scripts/rft_yaml_to_junit_xml

dist_noinst_SCRIPTS = \
	scripts/parseyaml.py

nobase_dist_config_DATA = \
	test_configs/osx.exclude \
        test_configs/eq_cq.test \
        test_configs/lat_bw.test \
	test_configs/tcp/all.test \
	test_configs/tcp/tcp.exclude \
	test_configs/tcp/io_uring.exclude \
        test_configs/sockets/all.test \
        test_configs/sockets/quick.test \
	test_configs/sockets/complete.test \
	test_configs/sockets/verify.test \
	test_configs/sockets/sockets.exclude \
        test_configs/udp/all.test \
        test_configs/udp/lat_bw.test \
        test_configs/udp/quick.test \
        test_configs/udp/functional.test \
        test_configs/udp/udp.exclude \
        test_configs/verbs/all.test \
        test_configs/verbs/quick.test \
	test_configs/verbs/verbs.exclude \
	test_configs/usnic/all.test \
	test_configs/usnic/quick.test \
	test_configs/psm2/all.test \
	test_configs/psm2/verify.test \
	test_configs/psm2/psm2.exclude \
	test_configs/psm3/all.test \
	test_configs/psm3/verify.test \
	test_configs/psm3/psm3.exclude \
	test_configs/ofi_rxm/tcp.test \
	test_configs/ofi_rxm/verbs.test \
	test_configs/ofi_rxm/ofi_rxm.exclude \
	test_configs/ofi_rxd/udp.test \
	test_configs/ofi_rxd/verbs.test \
	test_configs/ofi_rxd/ofi_rxd.exclude \
	test_configs/shm/all.test \
	test_configs/shm/shm.exclude \
	test_configs/shm/cuda.exclude \
	test_configs/shm/quick.test \
	test_configs/shm/verify.test \
	test_configs/sm2/quick.test \
	test_configs/sm2/verify.test \
	test_configs/efa/efa-neuron.exclude \
	test_configs/efa/efa.exclude \
	test_configs/lpp/all.test \
	test_configs/lpp/complete.test \
	test_configs/lpp/lpp.exclude \
	test_configs/lpp/quick.test \
	test_configs/lpp/verify.test \
	test_configs/ucx/all.test \
	test_configs/ucx/quick.test \
	test_configs/ucx/ucx.exclude \
	pytest/pytest.ini \
	pytest/options.yaml \
	pytest/conftest.py \
	pytest/common.py \
	pytest/default/test_av.py \
	pytest/default/test_cm.py \
	pytest/default/test_cntr.py \
	pytest/default/test_cq.py \
	pytest/default/test_dgram.py \
	pytest/default/test_dom.py \
	pytest/default/test_eq.py \
	pytest/default/test_getinfo.py \
	pytest/default/test_mr.py \
	pytest/default/test_inject_test.py \
	pytest/default/test_msg.py \
	pytest/default/test_multinode.py \
	pytest/default/test_multi_recv.py \
	pytest/default/test_rdm.py \
	pytest/default/test_recv_cancel.py \
	pytest/default/test_rma_bw.py \
	pytest/default/test_rma_pingpong.py \
	pytest/default/test_scalable_ep.py \
	pytest/default/test_shared_ctx.py \
	pytest/default/test_ubertest.py \
	pytest/default/test_unexpected_msg.py \
	pytest/default/test_sighandler.py \
	pytest/efa/conftest.py \
	pytest/efa/efa_common.py \
	pytest/efa/test_flood_peer.py \
	pytest/efa/test_from_default.py \
	pytest/efa/test_av.py \
	pytest/efa/test_cq.py \
	pytest/efa/test_setopt.py \
	pytest/efa/test_dgram.py \
	pytest/efa/test_rdm.py \
	pytest/efa/test_rma_bw.py \
	pytest/efa/test_rma_pingpong.py \
	pytest/efa/test_unexpected_msg.py \
	pytest/efa/test_multi_recv.py \
	pytest/efa/test_rnr.py \
	pytest/efa/test_remote_exit_early.py \
	pytest/efa/test_efa_info.py \
	pytest/efa/test_efa_protocol_selection.py \
	pytest/efa/test_efa_device_selection.py \
	pytest/efa/test_runt.py \
	pytest/efa/test_fork_support.py \
	pytest/efa/test_mr.py \
	pytest/efa/test_efa_shm_addr.py \
	pytest/efa/test_env.py \
	pytest/efa/test_multi_ep.py \
	pytest/efa/test_gda.py \
	pytest/shm/conftest.py \
	pytest/shm/shm_common.py \
	pytest/shm/test_av.py \
	pytest/shm/test_cntr.py \
	pytest/shm/test_cq.py \
	pytest/shm/test_dom.py \
	pytest/shm/test_eq.py \
	pytest/shm/test_getinfo.py \
	pytest/shm/test_mr.py \
	pytest/shm/test_multi_recv.py \
	pytest/shm/test_rdm.py \
	pytest/shm/test_rma_bw.py \
	pytest/shm/test_rma_pingpong.py \
	pytest/shm/test_ubertest.py \
	pytest/shm/test_unexpected_msg.py \
	pytest/shm/test_sighandler.py \
	pytest/sm2/conftest.py \
	pytest/sm2/sm2_common.py \
	pytest/sm2/test_rdm.py \
	pytest/sm2/test_av.py \
	pytest/sm2/test_cntr.py \
	pytest/sm2/test_cq.py \
	pytest/sm2/test_dom.py \
	pytest/sm2/test_eq.py \
	pytest/sm2/test_mr.py \
	pytest/sm2/test_sighandler.py \
	pytest/sm2/test_ubertest.py

noinst_LTLIBRARIES = libfabtests.la

libfabtests_la_SOURCES = \
	common/shared.c \
	common/hmem.c \
	common/hmem_cuda.c \
	common/hmem_rocr.c \
	common/hmem_ze.c \
	common/hmem_neuron.c \
	common/hmem_synapseai.c \
	common/ofi_atomic.c \
	include/ofi_atomic.h \
	include/shared.h \
	include/ft_list.h \
	include/hmem.h \
	include/jsmn.h \
	include/unix/osd.h \
	include/ft_osd.h

benchmarks_srcs = \
	benchmarks/benchmark_shared.h \
	benchmarks/benchmark_shared.c

unit_srcs = \
	include/unit_common.h \
	unit/common.c

if MACOS
if !HAVE_CLOCK_GETTIME
libfabtests_la_SOURCES += common/osx/osd.c
endif
endif

functional_fi_av_xfer_SOURCES = \
	functional/av_xfer.c
functional_fi_av_xfer_LDADD = libfabtests.la

functional_fi_msg_sockets_SOURCES = \
	functional/msg_sockets.c
functional_fi_msg_sockets_LDADD = libfabtests.la

functional_fi_msg_epoll_SOURCES = \
	functional/msg_epoll.c
functional_fi_msg_epoll_LDADD = libfabtests.la

functional_fi_msg_SOURCES = \
	functional/msg.c
functional_fi_msg_LDADD = libfabtests.la

functional_fi_rdm_SOURCES = \
	functional/rdm.c
functional_fi_rdm_LDADD = libfabtests.la

functional_fi_rdm_shared_av_SOURCES = \
	functional/rdm_shared_av.c
functional_fi_rdm_shared_av_LDADD = libfabtests.la

functional_fi_rdm_rma_event_SOURCES = \
	functional/rdm_rma_event.c
functional_fi_rdm_rma_event_LDADD = libfabtests.la

functional_fi_rdm_rma_trigger_SOURCES = \
	functional/rdm_rma_trigger.c
functional_fi_rdm_rma_trigger_LDADD = libfabtests.la

functional_fi_rdm_deferred_wq_SOURCES = \
	functional/rdm_deferred_wq.c
functional_fi_rdm_deferred_wq_LDADD = libfabtests.la

functional_fi_dgram_SOURCES = \
	functional/dgram.c
functional_fi_dgram_LDADD = libfabtests.la

functional_fi_mcast_SOURCES = \
	functional/mcast.c
functional_fi_mcast_LDADD = libfabtests.la

functional_fi_rdm_tagged_peek_SOURCES = \
	functional/rdm_tagged_peek.c
functional_fi_rdm_tagged_peek_LDADD = libfabtests.la

functional_fi_cq_data_SOURCES = \
	functional/cq_data.c
functional_fi_cq_data_LDADD = libfabtests.la

functional_fi_cm_data_SOURCES = \
	functional/cm_data.c
functional_fi_cm_data_LDADD = libfabtests.la

functional_fi_scalable_ep_SOURCES = \
	functional/scalable_ep.c
functional_fi_scalable_ep_LDADD = libfabtests.la

functional_fi_shared_ctx_SOURCES = \
	functional/shared_ctx.c
functional_fi_shared_ctx_LDADD = libfabtests.la

functional_fi_multi_ep_SOURCES = \
	functional/multi_ep.c
functional_fi_multi_ep_LDADD = libfabtests.la

functional_fi_multi_mr_SOURCES = \
	functional/multi_mr.c
functional_fi_multi_mr_LDADD = libfabtests.la

functional_fi_unexpected_msg_SOURCES = \
	functional/unexpected_msg.c
functional_fi_unexpected_msg_LDADD = libfabtests.la

functional_fi_unmap_mem_SOURCES = \
	functional/unmap_mem.c
functional_fi_unmap_mem_LDADD = libfabtests.la

functional_fi_rdm_multi_domain_SOURCES = \
	functional/rdm_multi_domain.c
functional_fi_rdm_multi_domain_LDADD = libfabtests.la

functional_fi_recv_cancel_SOURCES = \
	functional/recv_cancel.c
functional_fi_recv_cancel_LDADD = libfabtests.la

functional_fi_inject_test_SOURCES = \
	functional/inject_test.c
functional_fi_inject_test_LDADD = libfabtests.la

functional_fi_resmgmt_test_SOURCES = \
	functional/resmgmt_test.c
functional_fi_resmgmt_test_LDADD = libfabtests.la

functional_fi_rdm_atomic_SOURCES = \
	functional/rdm_atomic.c
functional_fi_rdm_atomic_LDADD = libfabtests.la

functional_fi_rdm_stress_SOURCES = \
	functional/rdm_stress.c
functional_fi_rdm_stress_LDADD = libfabtests.la

functional_fi_multi_recv_SOURCES = \
	functional/multi_recv.c
functional_fi_multi_recv_LDADD = libfabtests.la

functional_fi_flood_SOURCES = \
	 functional/flood.c
functional_fi_flood_LDADD = libfabtests.la

functional_fi_rdm_multi_client_SOURCES = \
	functional/rdm_multi_client.c
functional_fi_rdm_multi_client_LDADD = libfabtests.la

functional_fi_loopback_SOURCES = \
	functional/loopback.c
functional_fi_loopback_LDADD = libfabtests.la

benchmarks_fi_msg_pingpong_SOURCES = \
	benchmarks/msg_pingpong.c \
	$(benchmarks_srcs)
benchmarks_fi_msg_pingpong_LDADD = libfabtests.la

benchmarks_fi_msg_bw_SOURCES = \
	benchmarks/msg_bw.c \
	$(benchmarks_srcs)
benchmarks_fi_msg_bw_LDADD = libfabtests.la

benchmarks_fi_rma_bw_SOURCES = \
	benchmarks/rma_bw.c \
	$(benchmarks_srcs)
benchmarks_fi_rma_bw_LDADD = libfabtests.la

benchmarks_fi_dgram_pingpong_SOURCES = \
	benchmarks/dgram_pingpong.c \
	$(benchmarks_srcs)
benchmarks_fi_dgram_pingpong_LDADD = libfabtests.la

benchmarks_fi_rdm_cntr_pingpong_SOURCES = \
	benchmarks/rdm_cntr_pingpong.c \
	$(benchmarks_srcs)
benchmarks_fi_rdm_cntr_pingpong_LDADD = libfabtests.la

benchmarks_fi_rdm_pingpong_SOURCES = \
	benchmarks/rdm_pingpong.c \
	$(benchmarks_srcs)
benchmarks_fi_rdm_pingpong_LDADD = libfabtests.la

benchmarks_fi_rma_pingpong_SOURCES = \
	benchmarks/rma_pingpong.c \
	$(benchmarks_srcs)
benchmarks_fi_rma_pingpong_LDADD = libfabtests.la

benchmarks_fi_rdm_tagged_pingpong_SOURCES = \
	benchmarks/rdm_tagged_pingpong.c \
	$(benchmarks_srcs)
benchmarks_fi_rdm_tagged_pingpong_LDADD = libfabtests.la

benchmarks_fi_rdm_tagged_bw_SOURCES = \
	benchmarks/rdm_tagged_bw.c \
	$(benchmarks_srcs)
benchmarks_fi_rdm_tagged_bw_LDADD = libfabtests.la

benchmarks_fi_rdm_bw_SOURCES = \
	benchmarks/rdm_bw.c \
	$(benchmarks_srcs)
benchmarks_fi_rdm_bw_LDADD = libfabtests.la

benchmarks_fi_rdm_bw_mt_SOURCES = \
	benchmarks/rdm_bw_mt.c \
	$(benchmarks_srcs)
benchmarks_fi_rdm_bw_mt_LDADD = libfabtests.la

benchmarks_fi_rma_tx_completion_SOURCES = \
	benchmarks/rma_tx_completion.c \
	$(benchmarks_srcs)
benchmarks_fi_rma_tx_completion_LDADD = libfabtests.la


unit_fi_eq_test_SOURCES = \
	unit/eq_test.c \
	$(unit_srcs)
unit_fi_eq_test_LDADD = libfabtests.la

unit_fi_cq_test_SOURCES = \
	unit/cq_test.c \
	$(unit_srcs)
unit_fi_cq_test_LDADD = libfabtests.la

unit_fi_mr_test_SOURCES = \
	unit/mr_test.c \
	$(unit_srcs)
unit_fi_mr_test_LDADD = libfabtests.la

unit_fi_mr_cache_evict_SOURCES = \
	unit/mr_cache_evict.c \
	$(unit_srcs)
unit_fi_mr_cache_evict_LDADD = libfabtests.la

unit_fi_cntr_test_SOURCES = \
	unit/cntr_test.c \
	$(unit_srcs)
unit_fi_cntr_test_LDADD = libfabtests.la

unit_fi_av_test_SOURCES = \
	unit/av_test.c \
	$(unit_srcs)
unit_fi_av_test_LDADD = libfabtests.la

unit_fi_dom_test_SOURCES = \
	unit/dom_test.c \
	$(unit_srcs)
unit_fi_dom_test_LDADD = libfabtests.la

unit_fi_getinfo_test_SOURCES = \
	unit/getinfo_test.c \
	$(unit_srcs)
unit_fi_getinfo_test_LDADD = libfabtests.la

unit_fi_setopt_test_SOURCES = \
	unit/setopt_test.c \
	$(unit_srcs)
unit_fi_setopt_test_LDADD = libfabtests.la

ubertest_fi_ubertest_SOURCES = \
	ubertest/fabtest.h \
	ubertest/uber.c \
	ubertest/connect.c \
	ubertest/cq.c \
	ubertest/config.c \
	ubertest/domain.c \
	ubertest/ep.c \
	ubertest/xfer.c \
	ubertest/verify.c \
	ubertest/test_ctrl.c
ubertest_fi_ubertest_LDADD = libfabtests.la

multinode_fi_multinode_SOURCES = \
	multinode/src/harness.c \
	multinode/src/pattern.c \
	multinode/include/pattern.h \
	multinode/src/core.c \
	multinode/src/timing.c \
	multinode/include/timing.h \
	multinode/include/core.h

multinode_fi_multinode_LDADD = 	libfabtests.la

multinode_fi_multinode_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(srcdir)/multinode/include

multinode_fi_multinode_coll_SOURCES = \
	multinode/src/harness.c \
	multinode/src/core_coll.c \
	multinode/include/coll_test.h \
	multinode/include/core.h

multinode_fi_multinode_coll_LDADD = libfabtests.la

multinode_fi_multinode_coll_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(srcdir)/multinode/include

component_sock_test_SOURCES = \
	component/sock_test.c

component_sock_test_LDADD = libfabtests.la

component_sock_test_CFLAGS = \
	$(AM_CFLAGS)

if HAVE_ZE_DEVEL
if HAVE_VERBS_DEVEL
component_dmabuf_rdma_xe_rdmabw_SOURCES = \
	component/dmabuf-rdma/rdmabw-xe.c \
	component/dmabuf-rdma/util.c \
	component/dmabuf-rdma/util.h \
	component/dmabuf-rdma/xe.c \
	component/dmabuf-rdma/xe.h \
	component/dmabuf-rdma/dmabuf_reg.c \
	component/dmabuf-rdma/dmabuf_reg.h

component_dmabuf_rdma_xe_rdmabw_LDADD = libfabtests.la

component_dmabuf_rdma_xe_rdmabw_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(srcdir)/component/dmabuf-rdma

component_dmabuf_rdma_xe_mr_reg_SOURCES = \
	component/dmabuf-rdma/mr-reg-xe.c \
	component/dmabuf-rdma/util.h \
	component/dmabuf-rdma/xe.c \
	component/dmabuf-rdma/xe.h \
	component/dmabuf-rdma/dmabuf_reg.c \
	component/dmabuf-rdma/dmabuf_reg.h

component_dmabuf_rdma_xe_mr_reg_LDADD = libfabtests.la

component_dmabuf_rdma_xe_mr_reg_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(srcdir)/component/dmabuf-rdma
endif HAVE_VERBS_DEVEL

component_dmabuf_rdma_fi_xe_rdmabw_SOURCES = \
	component/dmabuf-rdma/fi-rdmabw-xe.c \
	component/dmabuf-rdma/ofi_ctx_pool.h \
	component/dmabuf-rdma/util.c \
	component/dmabuf-rdma/util.h \
	component/dmabuf-rdma/xe.c \
	component/dmabuf-rdma/xe.h \
	component/dmabuf-rdma/dmabuf_reg.c \
	component/dmabuf-rdma/dmabuf_reg.h

component_dmabuf_rdma_fi_xe_rdmabw_LDADD = libfabtests.la

component_dmabuf_rdma_fi_xe_rdmabw_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(srcdir)/component/dmabuf-rdma

component_dmabuf_rdma_fi_xe_mr_reg_SOURCES = \
	component/dmabuf-rdma/fi-mr-reg-xe.c \
	component/dmabuf-rdma/util.h \
	component/dmabuf-rdma/xe.c \
	component/dmabuf-rdma/xe.h \
	component/dmabuf-rdma/dmabuf_reg.c \
	component/dmabuf-rdma/dmabuf_reg.h

component_dmabuf_rdma_fi_xe_mr_reg_LDADD = libfabtests.la

component_dmabuf_rdma_fi_xe_mr_reg_CFLAGS = \
	$(AM_CFLAGS) \
	-I$(srcdir)/component/dmabuf-rdma

component_dmabuf_rdma_xe_memcopy_SOURCES = \
	component/dmabuf-rdma/memcopy-xe.c \
	component/dmabuf-rdma/util.c \
	component/dmabuf-rdma/util.h

component_dmabuf_rdma_xe_memcopy_LDADD = libfabtests.la

component_dmabuf_rdma_xe_memcopy_CFLAGS = \
	$(AM_CFLAGS)
endif HAVE_ZE_DEVEL

regression_sighandler_test_SOURCES = \
	regression/sighandler_test.c

regression_sighandler_test_LDADD = libfabtests.la

regression_sighandler_test_CFLAGS = \
	$(AM_CFLAGS)

common_check_hmem_SOURCES = \
	common/check_hmem.c

common_check_hmem_LDADD = libfabtests.la

common_checK_hmem_CFLAGS = \
	$(AM_CFLAGS)

real_man_pages = \
	 man/man7/fabtests.7

dummy_man_pages = \
	man/man1/fi_av_xfer.1 \
	man/man1/fi_cm_data.1 \
	man/man1/fi_cq_data.1 \
	man/man1/fi_dgram.1 \
	man/man1/fi_inj_complete.1 \
	man/man1/fi_mcast.1 \
	man/man1/fi_msg.1 \
	man/man1/fi_msg_epoll.1 \
	man/man1/fi_msg_sockets.1 \
	man/man1/fi_multi_ep.1 \
	man/man1/fi_multi_mr.1 \
	man/man1/fi_rdm.1 \
	man/man1/fi_rdm_atomic.1 \
	man/man1/fi_rdm_deferred_wq.1 \
	man/man1/fi_rdm_multi_domain.1 \
	man/man1/fi_multi_recv.1 \
	man/man1/fi_rdm_rma_event.1 \
	man/man1/fi_rdm_rma_trigger.1 \
	man/man1/fi_rdm_shared_av.1 \
	man/man1/fi_rdm_tagged_peek.1 \
	man/man1/fi_rdm_stress.1 \
	man/man1/fi_recv_cancel.1 \
	man/man1/fi_resmgmt_test.1 \
	man/man1/fi_scalable_ep.1 \
	man/man1/fi_shared_ctx.1 \
	man/man1/fi_unexpected_msg.1 \
	man/man1/fi_unmap_mem.1 \
	man/man1/fi_dgram_pingpong.1 \
	man/man1/fi_msg_bw.1 \
	man/man1/fi_msg_pingpong.1 \
	man/man1/fi_rdm_cntr_pingpong.1 \
	man/man1/fi_rdm_pingpong.1 \
	man/man1/fi_rdm_tagged_bw.1 \
	man/man1/fi_rdm_tagged_pingpong.1 \
	man/man1/fi_rma_bw.1 \
	man/man1/fi_av_test.1 \
	man/man1/fi_cntr_test.1 \
	man/man1/fi_cq_test.1 \
	man/man1/fi_dom_test.1 \
	man/man1/fi_eq_test.1 \
	man/man1/fi_getinfo_test.1 \
	man/man1/fi_mr_test.1 \
	man/man1/fi_flood.1 \
	man/man1/fi_rdm_multi_client.1 \
	man/man1/fi_ubertest.1 \
	man/man1/fi_efa_ep_rnr_retry.1

nroff:
	@for file in $(real_man_pages); do \
            source=`echo $$file | sed -e 's@/man[0-9]@@'`; \
            perl $(top_srcdir)/config/md2nroff.pl --source=$(top_srcdir)/$$source.md; \
        done

include prov/efa/Makefile.include
if !MACOS
include prov/lpp/Makefile.include
endif

man_MANS = $(real_man_pages) $(dummy_man_pages)

EXTRA_DIST = \
	fabtests.spec.in $(real_man_pages) $(dummy_man_pages)

dist-hook: fabtests.spec
	cp fabtests.spec $(distdir)

test:
	./scripts/runfabtests.sh -vvv -S $(os_excludes) -f ./test_configs/sockets/sockets.exclude sockets
	./scripts/runfabtests.sh -vvv -S $(os_excludes) -f ./test_configs/udp/udp.exclude udp
