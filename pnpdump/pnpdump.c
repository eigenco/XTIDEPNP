#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SFN
#include <isapnp/pnp_acce.h>
#else
#include <isapnp/pnp-access.h>
#endif

int
main(int argc, char **argv) {
  return (pnpdump_main(argc, argv));
}
