#include "ccnx_util.h"

ccn_charbuf *
ccn_charbuf_dup(const ccn_charbuf *target) {
	if (target == NULL) {
		return NULL;
	}

	ccn_charbuf *temp = ccn_charbuf_create();
	ccn_charbuf_reserve(temp, target->length);
	memcpy(temp->buf, target->buf, target->length);
	temp->length = target->length;
	return temp;
}

int
get_timestamp_in_seconds(const ccn_charbuf *target, const ccn_parsed_ContentObject &pco, time_t *timestamp){
	const unsigned char *result = NULL;
	size_t size = 0;
	int res = ccn_ref_tagged_BLOB(CCN_DTAG_Timestamp, target->buf, pco.offset[CCN_PCO_B_Timestamp],pco.offset[CCN_PCO_E_Timestamp], &result, &size);
	if (res < 0) {
		return res;
	}

	long time = 0;
	// copy the bits in Timestamp part into long
	// apparently it is in network order, i.e. big-endian
	// so we need to convert it to little-endian
	unsigned char *temp = (unsigned char *) &time;
	for (int i = 0; i < size; i++) {
		temp[size - 1 - i] = result[i];
	}

	// discard the fraction part (12 bits)
	time = (time >> 12);

	*timestamp = (time_t) time;

	return 0;
}

int
get_freshness_in_days(const ccn_charbuf *target, const ccn_parsed_ContentObject &pco) {
	int days = 0;
	days = ccn_fetch_tagged_nonNegativeInteger(CCN_DTAG_FreshnessSeconds, target->buf, pco.offset[CCN_PCO_B_FreshnessSeconds], pco.offset[CCN_PCO_E_FreshnessSeconds]);
	return days;
}

int
contain_key_name(const unsigned char *ccnb, ccn_parsed_ContentObject *pco) {
	if (pco->offset[CCN_PCO_B_KeyLocator] == pco->offset[CCN_PCO_E_KeyLocator])
		return -1;
	
	ccn_buf_decoder decoder;
	ccn_buf_decoder *d;
	d = ccn_buf_decoder_start(&decoder, ccnb + pco->offset[CCN_PCO_B_Key_Certificate_KeyName], pco->offset[CCN_PCO_E_Key_Certificate_KeyName] - pco->offset[CCN_PCO_B_Key_Certificate_KeyName]);
	if (ccn_buf_match_dtag(d, CCN_DTAG_KeyName))
		return 1;
	
	return -1;
}

ccn_charbuf *
get_key_name(const unsigned char *ccnb, ccn_parsed_ContentObject *pco) {
	const unsigned char *key_buf = NULL;
	size_t key_len = 0;

	ccn_ref_tagged_BLOB(CCN_DTAG_KeyName, ccnb, pco->offset[CCN_PCO_B_Key_Certificate_KeyName], pco->offset[CCN_PCO_E_Key_Certificate_KeyName], &key_buf, &key_len);

	ccn_charbuf *key_name = ccn_charbuf_create();
	ccn_charbuf_reserve(key_name, key_len);
	memcpy(key_name->buf, key_buf, key_len);
	key_name->length = key_len;

	return key_name;
}
