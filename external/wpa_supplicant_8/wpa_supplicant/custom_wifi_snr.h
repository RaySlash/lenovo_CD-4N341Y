/*
 * WPA Supplicant - Custom SNR function used for sorting
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Returns a custom "SNR" to alter sorting behavior of wireless APs.
int get_custom_snr(int signal_level, int noise, int frequency);

#ifdef __cplusplus
}
#endif
