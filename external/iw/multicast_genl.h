#ifndef __MULTICAST_GENL_H_
#define __MULTICAST_GENL_H_

struct nl_sock;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

int nl_get_multicast_id(struct nl_sock* sock, const char* family, const char* group);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __MULTICAST_GENL_H_
