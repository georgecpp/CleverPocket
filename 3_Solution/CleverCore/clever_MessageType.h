#pragma once
#include "clever_inclusions.h"
namespace clever
{
	enum class MessageType : uint32_t
	{
		ServerAccept,
		ServerDeny,
		ServerPing,
		MessageAll,
		ServerMessage,
		RegisterRequest,
		LoginRequest,
		LoginRememeberedRequest,
		RememberMeRequest,
	};
}