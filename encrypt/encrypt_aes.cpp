#include "../util.h"
#include "encrypt_aes.h"

static void aes_valid_key(std::vector<uint8_t> key){
	if(key.size() != 192/8){
		print("AES key is not 192 bit", P_ERR);
	}
}

static std::vector<uint8_t> aes_raw_encrypt(std::vector<uint8_t> plaintext, std::vector<uint8_t> key, std::vector<uint8_t> iv){
	std::vector<uint8_t> retval(plaintext.size()*2, 0);
	EVP_CIPHER_CTX *ctx = nullptr;
	int32_t len;
	int32_t ciphertext_len;
	if(!(ctx = EVP_CIPHER_CTX_new())){
		print("can't create EVP_CIPHER_CTX", P_ERR);
	}
	if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_192_cbc(), nullptr, key.data(), iv.data())){
		print("can't initialize AES", P_ERR);
	}
	if(1 != EVP_EncryptUpdate(ctx, retval.data(), &len, plaintext.data(), plaintext.size())){
		print("can't update AES", P_ERR);
	}
	ciphertext_len = len;
	if(1 != EVP_EncryptFinal_ex(ctx, retval.data(), &len)){
		print("can't encrypt AES", P_ERR);
	}
	ciphertext_len += len;
	retval = std::vector<uint8_t>(
		retval.begin(),
		retval.begin()+ciphertext_len);
	EVP_CIPHER_CTX_free(ctx);
	return retval;
}

static void aes_error_printer(){
	ERR_print_errors_fp(stderr);
}

static std::vector<uint8_t> aes_raw_decrypt(std::vector<uint8_t> ciphertext, std::vector<uint8_t> key, std::vector<uint8_t> iv){
	std::vector<uint8_t> retval(ciphertext.size()*2, 0);
	EVP_CIPHER_CTX *ctx = nullptr;
	int len;
	int plaintext_len;
	P_V(key.size(), P_NOTE);
	P_V(iv.size(), P_NOTE);
	if(!(ctx = EVP_CIPHER_CTX_new())){
		aes_error_printer();
		print("can't create EVP_CIPHER_CTX", P_ERR);
	}
	if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_192_cbc(), nullptr, key.data(), iv.data())){
		aes_error_printer();
		print("couldn't initialize AES decryption", P_ERR);
	}
	if(1 != EVP_DecryptUpdate(ctx, retval.data(), &len, ciphertext.data(), ciphertext.size())){
		aes_error_printer();
		print("couldn't update AES decryption", P_ERR);
	}else{
		retval = std::vector<uint8_t>(
			retval.begin(),
			retval.begin()+len);
	}
	plaintext_len = len;
	if(1 != EVP_DecryptFinal_ex(ctx, retval.data(), &len)){
		P_V(len, P_NOTE);
		P_V(retval.size(), P_NOTE);
		aes_error_printer();
		print("couldn't decrypt AES", P_ERR);
	}
	plaintext_len += len;
	retval = std::vector<uint8_t>(
		retval.begin(),
		retval.begin()+plaintext_len);
	EVP_CIPHER_CTX_free(ctx);
	return retval;
}

std::vector<uint8_t> aes::encrypt(std::vector<uint8_t> data,
				  std::vector<uint8_t> key){
	aes_valid_key(key);
	std::vector<uint8_t> retval((key.size())+(data.size()*2), 0); // enough
	std::vector<uint8_t> iv;
	for(uint64_t i = 0;i < key.size();i++){
		iv.push_back((uint8_t)true_rand(0, 255));
	}
	retval.insert(
		retval.end(),
		iv.begin(),
		iv.end());
	std::vector<uint8_t> ciphertext =
		aes_raw_encrypt(
			data,
			key,
			iv);
	retval.insert(
		retval.end(),
		ciphertext.begin(),
		ciphertext.end());
	return retval;
}

std::vector<uint8_t> aes::decrypt(std::vector<uint8_t> data,
				  std::vector<uint8_t> key){
	aes_valid_key(key);
	std::vector<uint8_t> retval;
	std::vector<uint8_t> iv(
		data.begin(),
		data.begin()+(key.size()));
	data = std::vector<uint8_t>(
		data.begin()+(key.size()),
		data.end());
	retval =
		aes_raw_decrypt(
			data,
			key,
			iv);
	return retval;
}
