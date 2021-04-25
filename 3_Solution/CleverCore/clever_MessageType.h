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
		SendEmailForgotPasswordRequest,
		VerifyCodeForgotPasswordRequest,
		UpdatePasswordRequest,
		LogoutRememberedRequest,
		AddCardUsernameRequest,
		AddCardPATRequest,
		ServerAddCardResponse,
		UserGetCardRequest,
		PATGetCardRequest,
		ServerGetCardsResponse,
		ServerAddFundsResponse,
		AddCardFundsPATRequest,
		AddCardFundsUsernameRequest,
	};
}