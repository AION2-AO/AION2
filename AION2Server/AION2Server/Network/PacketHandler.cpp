#include "pch.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "ItemData.h"
#include "Room.h"
#include "Player.h"
#include "ObjectUtils.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool PacketHandler::HandleSignUp(PacketSessionRef& session, Protocol::C_SignUpPacket& pkt)
{
	std::cout << "SignUp Request: ID(" << pkt.id() << ") PW(" << pkt.password() << ") Class(" << static_cast<int32>(pkt.classtype()) << ")" << std::endl;

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind<3, 1> dbBind(*dbConnect, L"{CALL sp_RegisterUser(?, ?, ?)}");

	WCHAR wId[51] = { 0, };
	WCHAR wPassword[51] = { 0, };

	::mbstowcs_s(nullptr, wId, 51, pkt.id().c_str(), _TRUNCATE);
	::mbstowcs_s(nullptr, wPassword, 51, pkt.password().c_str(), _TRUNCATE);

	dbBind.BindParam(0, wId);
	dbBind.BindParam(1, wPassword);

	int32 classTypeInt = static_cast<int32>(pkt.classtype());
	dbBind.BindParam(2, classTypeInt);

	int32 resultCode = 0;
	dbBind.BindCol(0, resultCode);

	if (dbBind.Execute())
	{
		if (dbBind.Fetch())
		{
			std::cout << "ResultCode : " << resultCode << std::endl;
		}
	}

	GDBConnectionPool->Push(dbConnect);

	Protocol::S_SignUpResultPacket resultPkt;
	resultPkt.set_success(resultCode == 0);

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(resultPkt);
	session->Send(sendBuffer);

	return true;
}

bool PacketHandler::HandleLogin(PacketSessionRef& session, Protocol::C_LoginPacket& pkt)
{
	std::cout << "Login Request: ID(" << pkt.id() << ") PW(" << pkt.password() << ")" << std::endl;
	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind<2, 9> dbBind(*dbConnect, L"{CALL sp_LogIn(?, ?)}");

	WCHAR wId[51] = { 0, };
	WCHAR wPassword[51] = { 0, };

	::mbstowcs_s(nullptr, wId, 51, pkt.id().c_str(), _TRUNCATE);
	::mbstowcs_s(nullptr, wPassword, 51, pkt.password().c_str(), _TRUNCATE);

	dbBind.BindParam(0, wId);
	dbBind.BindParam(1, wPassword);

	int32 errorCode = -1;
	int32 playerClass = 0;
	int32 exp = 0;
	int32 gold = 0;
	int32 hp = 0;
	int32 itemInstanceId = 0;
	int32 itemTemplateId = 0;
	int32 slotIndex = 0;
	int32 itemCount = 0;

	std::wcout.imbue(std::locale("kor"));
	dbBind.BindCol(0, errorCode);
	dbBind.BindCol(1, playerClass);
	dbBind.BindCol(2, exp);
	dbBind.BindCol(3, gold);
	dbBind.BindCol(4, hp);
	dbBind.BindCol(5, itemInstanceId);
	dbBind.BindCol(6, itemTemplateId);
	dbBind.BindCol(7, slotIndex);
	dbBind.BindCol(8, itemCount);

	Protocol::S_ItemDataPacket itemPkt;
	bool isFirstRow = true;
	bool loginSuccess = false;

	if (dbBind.Execute())
	{
		while (dbBind.Fetch())
		{
			if (errorCode == 1)
			{
				std::cout << "Login Fail: Id or Password" << std::endl;
				break;
			}
			loginSuccess = true;
			if (isFirstRow)
			{
				GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
				PlayerRef player = ObjectUtils::CreatePlayer(gameSession);
				player->SetPlayerInfo(playerClass, exp, gold, hp);
				isFirstRow = false;
			}
			if (itemInstanceId != 0)
			{
				Protocol::ItemData* item = itemPkt.add_playeritems();

				item->set_iteminstancedid(itemInstanceId);
				item->set_itemtemplateid(itemTemplateId);
				item->set_slotindex(slotIndex);
				item->set_count(itemCount);
			}
		}
	}

	GDBConnectionPool->Push(dbConnect);

	Protocol::S_LoginSuccessPacket loginPkt;

	if (loginSuccess)
	{
		std::cout << " Login Success!Inventory Item Count : " << itemPkt.playeritems_size() << std::endl;

		GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
		PlayerRef player = gameSession->_player;
		Protocol::PlayerInfo* playerInfo = loginPkt.mutable_playerinfo();
		playerInfo->set_playerclass(static_cast<Protocol::ClassType>(player->_class));
		playerInfo->set_playerid(player->_playerId);
		loginPkt.set_gold(player->_gold);
		loginPkt.set_exp(player->_exp);
		loginPkt.set_hp(player->_hp);

		SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(loginPkt);
		session->Send(sendBuffer);

		SendBufferRef itemSendBuffer = PacketHandler::MakeSendBuffer(itemPkt);
		session->Send(itemSendBuffer);

		GRoom->DoAsync(&Room::AddPlayer, player);
	}

	else
	{
		Protocol::S_LoginFailPacket failPkt;
		SendBufferRef failSendBuffer = PacketHandler::MakeSendBuffer(failPkt);
		session->Send(failSendBuffer);
	}

	return true;
}

bool PacketHandler::HandleMapComplete(PacketSessionRef& session, Protocol::C_MapLoadCompletePacket& pkt)
{
	std::cout << "Handle Map Complete\n";
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	GRoom->DoAsync(&Room::HandleEnterPlayer, player);

	return false;
}

bool PacketHandler::HandleMove(PacketSessionRef& session, Protocol::C_MovePacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->_player;
	if (player == nullptr)
		return false;

	GRoom->DoAsync(&Room::HandleMove, pkt, player);

	return true;
}
