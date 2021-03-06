#define BOOST_TEST_MODULE testvkey
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <iostream>
#include "../src/vkey.hpp"
#include <cstdio>

using namespace std;
using namespace VKey;
using namespace boost;

/*
ccn_charbuf *readKey() {
	unsigned char *keydata = NULL;
	size_t kd_size, len;
	FILE *fp = fopen("tests/keys/root.pem", "r");
	BOOST_CHECK(fp != NULL);
	X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
	fclose(fp);
	BOOST_CHECK(cert !=NULL);
    kd_size = i2d_X509_PUBKEY(X509_get_X509_PUBKEY(cert), &keydata);
	BOOST_CHECK(kd_size > 0);
	ccn_charbuf *key = ccn_charbuf_create();
	ccn_charbuf_reserve(key, kd_size);
	memcpy(key->buf, keydata, kd_size);
	key->length = kd_size;

	return key;
}
*/

/*
BOOST_AUTO_TEST_CASE(KeyObjectTest)
{

	ccn_charbuf *key = readKey();	
	time_t now = time(NULL);
	int freshness = 1;
	now -= freshness * 60 * 60 * 24;
	now += 1;

	CcnxKeyObjectPtr ptr(new CcnxKeyObject("/vkey/test/key", key, now, freshness));	
	
	BOOST_CHECK_EQUAL(ptr->getKeyName(), "/vkey/test/key");
	BOOST_CHECK_EQUAL(ptr->getTimestamp(), now);
	BOOST_CHECK_EQUAL(ptr->getFreshness(), freshness);
	BOOST_CHECK(ptr->getCcnPKey() != NULL);
	BOOST_CHECK(!ptr->expired());

	sleep(2);

	BOOST_CHECK(ptr->expired());
	ccn_charbuf_destroy(&key);
	key = NULL;
}
*/

/*
BOOST_AUTO_TEST_CASE(Sqlite3Manager)
{
	KeyDBManager * dbm = new SqliteKeyDBManager();
	ccn_charbuf *key = readKey();	

	time_t now = time(NULL);
	int freshness = 1;
	now -= freshness * 60 * 60 * 24;
	now += 1;

	CcnxKeyObjectPtr ptr(new CcnxKeyObject("/vkey/test/key1", key, now, freshness));	

	dbm->update();
	dbm->insert(ptr);
	const CcnxKeyObjectPtr dbPtr = dbm->query(ptr->getKeyName());
	BOOST_CHECK(dbPtr != CcnxKeyObject::Null);
	BOOST_CHECK(dbPtr->getKeyName() == ptr->getKeyName());
	BOOST_CHECK(dbPtr->getTimestamp() == ptr->getTimestamp());
	BOOST_CHECK(dbPtr->getFreshness() == ptr->getFreshness());
	ccn_charbuf *skey = ptr->getKey();
	ccn_charbuf *dbKey = dbPtr->getKey();
	BOOST_CHECK(skey->length == dbKey->length);
	BOOST_CHECK_EQUAL(memcmp(skey->buf, dbKey->buf, skey->length), 0);
	
	sleep(2);

	dbm->update();
	const CcnxKeyObjectPtr nullPtr = dbm->query(ptr->getKeyName());
	BOOST_CHECK_EQUAL(nullPtr, CcnxKeyObject::Null);

	ccn_charbuf_destroy(&key);
	ccn_charbuf_destroy(&skey);
	ccn_charbuf_destroy(&dbKey);
}
*/

BOOST_AUTO_TEST_CASE(KeyFetcher) {
	system("./tests/init.sh");
	ccn_charbuf *name = ccn_charbuf_create();
	ccn_name_from_uri(name, "/vkey/test/WEapbsIN-BQAfKM4rjlxpkt7f6o=");
	const CcnxKeyObjectPtr ptr = CcnxOneTimeKeyFetcher::fetch(name);
	BOOST_CHECK(ptr != CcnxKeyObject::Null);
	BOOST_CHECK_EQUAL(ptr->getFreshness(), 1);
	BOOST_CHECK_EQUAL(ptr->getKeyName(), "/vkey/test/WEapbsIN-BQAfKM4rjlxpkt7f6o=");
	ccn_charbuf_destroy(&name);
}

BOOST_AUTO_TEST_CASE(Verifier) {
	
	ccn *h = ccn_create();
	ccn_connect(h, NULL);
	ccn_charbuf *name = ccn_charbuf_create();
	ccn_name_from_uri(name, "/vkey/test/site/node/content/info");
	ccn_charbuf *result = ccn_charbuf_create();
	ccn_parsed_ContentObject pco = {0};
	int get_flags = 0;
	// no need for ccnd to verify key
	get_flags |= CCN_GET_NOKEYWAIT;
	ccn_get(h, name, NULL, 500, result, &pco, NULL, get_flags);

	SigVerifier *verifier = SigVerifier::getInstance();
	BOOST_CHECK(verifier->verify(result->buf, &pco));

	// verify again, this time we'll be using cached keys
	BOOST_CHECK(verifier->verify(result->buf, &pco));
	ccn_charbuf_destroy(&name);
	ccn_charbuf_destroy(&result);
	ccn_destroy(&h);
}
