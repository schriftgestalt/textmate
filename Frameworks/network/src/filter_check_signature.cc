#include "filter_check_signature.h"
#include <cf/src/cf.h>
#include <text/src/decode.h>
#include <text/src/format.h>
#include <oak/debug.h>

namespace network
{
	check_signature_t::check_signature_t (key_chain_t const& keyChain, std::string const& signeeHeader, std::string const& signatureHeader) : _key_chain(keyChain), _signee_header(signeeHeader), _signature_header(signatureHeader), _data(nullptr)
	{
	}

	check_signature_t::~check_signature_t ()
	{
		if(_data)
			CFRelease(_data);
	}

	bool check_signature_t::setup ()
	{
		if(_data)
			CFRelease(_data);

		return _data = CFDataCreateMutable(kCFAllocatorDefault, 0);
	}

	bool check_signature_t::receive_header (std::string const& header, std::string const& value)
	{
		if(header == _signee_header)
			_signee = value;
		else if(header == _signature_header)
			_signature = value;
		return true;
	}

	bool check_signature_t::receive_data (char const* buf, size_t len)
	{
		CFDataAppendBytes(_data, (const UInt8*)buf, len);
		return true;
	}

	bool check_signature_t::receive_end (std::string& error)
	{
		if(_signee == NULL_STR)
			return (error = "Missing signee."), false;
		if(_signature == NULL_STR)
			return (error = "Missing signature."), false;

		bool res = false;

		if(key_chain_t::key_ptr key = _key_chain.find(_signee))
		{
			std::string signature = decode::base64(_signature);

			CFErrorRef err = nullptr;
			CFDataRef sig_data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)signature.data(), signature.size(), kCFAllocatorNull);
			if(SecTransformRef verifier = SecVerifyTransformCreate(*key, sig_data, &err))
			{
				if(SecTransformSetAttribute(verifier, kSecTransformInputAttributeName, _data, &err))
				{
					res = SecTransformExecute(verifier, &err) == kCFBooleanTrue;

					if(!res)
						error = text::format("Bad signature.");
				}
				else
					error = text::format("Error setting transform input: ‘%s’.", cf::to_s(err).c_str());

				CFRelease(verifier);
			}
			else
				error = text::format("Error creating verify transform: ‘%s’.", cf::to_s(err).c_str());

			if(sig_data)
				CFRelease(sig_data);
		}
		else
			error = text::format("Unknown signee: ‘%s’.", _signee.c_str());

		return res;
	}

	std::string check_signature_t::name ()
	{
		return "signature";
	}

} /* network */
