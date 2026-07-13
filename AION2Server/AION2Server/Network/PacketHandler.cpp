#include "pch.h"
#include "PacketHandler.h"
#include "GameSession.h"
#include "Session/DedicatedSession.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "Room.h"
#include "Dungeon.h"
#include "Player.h"
#include "ObjectUtils.h"
#include "RedisManager.h"

std::wstring Utf8ToUtf16(const std::string& utf8Str)
{
	if (utf8Str.empty()) return L"";
	int32 sizeNeeded = ::MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int32>(utf8Str.size()), NULL, 0);
	std::wstring utf16Str(sizeNeeded, 0);
	::MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int32>(utf8Str.size()), &utf16Str[0], sizeNeeded);
	return utf16Str;
}

std::string Utf16ToUtf8(const WCHAR* utf16Str)
{
	if (utf16Str == nullptr || *utf16Str == L'\0') return "";
	int32 sizeNeeded = ::WideCharToMultiByte(CP_UTF8, 0, utf16Str, -1, NULL, 0, NULL, NULL);
	std::string utf8Str(sizeNeeded - 1, 0); // null 문자 제외
	::WideCharToMultiByte(CP_UTF8, 0, utf16Str, -1, &utf8Str[0], sizeNeeded, NULL, NULL);
	return utf8Str;
}

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
	DBBind<3, 2> dbBind(*dbConnect, L"{CALL sp_RegisterUser(?, ?, ?)}");

	std::wstring wId = Utf8ToUtf16(pkt.id());
	std::wstring wPassword = Utf8ToUtf16(pkt.password());

	dbBind.BindParam(0, wId.c_str());
	dbBind.BindParam(1, wPassword.c_str());

	int32 classTypeInt = static_cast<int32>(pkt.classtype());
	dbBind.BindParam(2, classTypeInt);

	int32 resultCode = 0;
	int32 networkId = 0;

	dbBind.BindCol(0, resultCode);
	dbBind.BindCol(1, networkId);

	if (dbBind.Execute())
	{
		if (dbBind.Fetch())
		{
			std::cout << "ResultCode : " << resultCode << std::endl;
		}
	}
	GDBConnectionPool->Push(dbConnect);

	// -1이면 실패, 1이면 성공
	Protocol::S_SignUpResultPacket resultPkt;
	resultPkt.set_success((resultCode == 1));

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(resultPkt);
	session->Send(sendBuffer);

	if (resultCode != 1) return false;

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = ObjectUtils::CreatePlayer(gameSession);
	player->SetPlayerInfo(networkId, pkt.classtype(), 0, 100, 100);
	return true;
}

bool PacketHandler::HandleSetNickname(PacketSessionRef& session, Protocol::C_SetNicknamePacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	int32 playerId = player->GetId();

	std::wstring playerNickname = Utf8ToUtf16(pkt.nickname());

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind<2, 1> dbBind(*dbConnect, L"{CALL sp_SetNickname(?, ?)}");

	dbBind.BindParam(0, playerId);
	dbBind.BindParam(1, playerNickname.c_str());

	int8 result = 0;
	dbBind.BindCol(0, result);

	if (dbBind.Execute())
	{
		if (dbBind.Fetch())
		{
			std::cout << "Nickname ResultCode : " << result << std::endl;

		}
	}

	Protocol::S_SetNicknamePacket nicknamePacket;
	nicknamePacket.set_issucceed(result);
	SendBufferRef nickBuffer = PacketHandler::MakeSendBuffer(nicknamePacket);
	session->Send(nickBuffer);

	if (result != 0) return false;

	std::string nickname = Utf16ToUtf8(playerNickname.c_str());
	player->SetName(nickname);

	Protocol::S_LoginSuccessPacket loginPkt;

	Protocol::PlayerInfo* playerInfo = loginPkt.mutable_playerinfo();
	playerInfo->set_playerclass(static_cast<Protocol::ClassType>(player->GetClass()));
	playerInfo->set_playerid(player->_playerId);
	playerInfo->set_playernickname(player->GetName());
	loginPkt.set_gold(player->_gold);
	loginPkt.set_exp(player->_exp);
	loginPkt.set_hp(player->_hp);

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

	GRoom->DoAsync(&Room::AddPlayer, player);
}

bool PacketHandler::HandleLogin(PacketSessionRef& session, Protocol::C_LoginPacket& pkt)
{
	std::cout << "Login Request: ID(" << pkt.id() << ") PW(" << pkt.password() << ")" << std::endl;

	// 더미유저 우회 로직 (DB 타지 않고 즉석 가상 로그인 성공 처리)
	if (pkt.id().rfind("dummy_user_", 0) == 0)
	{
		std::string szNickname = pkt.id();
		uint64 playerId = 100000 + std::stoull(pkt.id().substr(11));

		GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
		PlayerRef player = ObjectUtils::CreatePlayer(gameSession);
		player->SetPlayerInfo(playerId, static_cast<Protocol::ClassType>(1), 100, 1000, 100);
		player->SetName(szNickname);

		Protocol::S_LoginSuccessPacket loginPkt;
		Protocol::PlayerInfo* playerInfo = loginPkt.mutable_playerinfo();
		playerInfo->set_playerclass(static_cast<Protocol::ClassType>(player->_class));
		playerInfo->set_playerid(player->_playerId);
		playerInfo->set_playernickname(player->GetName());
		loginPkt.set_gold(player->_gold);
		loginPkt.set_exp(player->_exp);
		loginPkt.set_hp(player->_hp);

		SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(loginPkt);
		session->Send(sendBuffer);

		// 빈 아이템 패킷 전송
		Protocol::S_ItemDataPacket itemPkt;
		SendBufferRef itemSendBuffer = PacketHandler::MakeSendBuffer(itemPkt);
		session->Send(itemSendBuffer);

		GRoom->DoAsync(&Room::AddPlayer, player);

		std::cout << "Dummy Login Bypass Success! ID: " << szNickname << std::endl;
		return true;
	}

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind<2, 11> dbBind(*dbConnect, L"{CALL sp_LogIn(?, ?)}");

	std::wstring wId = Utf8ToUtf16(pkt.id());
	std::wstring wPassword = Utf8ToUtf16(pkt.password());

	dbBind.BindParam(0, wId.c_str());
	dbBind.BindParam(1, wPassword.c_str());

	int64 playerId = -1;
	int32 errorCode = -1;
	int32 playerClass = 0;
	int32 exp = 0;
	int32 gold = 0;
	int32 hp = 0;
	int32 itemInstanceId = 0;
	int32 itemTemplateId = 0;
	int32 slotIndex = 0;
	int32 itemCount = 0;
	WCHAR nickname[51] = { 0, };

	std::wcout.imbue(std::locale("kor"));

	dbBind.BindCol(0, errorCode);
	dbBind.BindCol(1, playerId);
	dbBind.BindCol(2, nickname);
	dbBind.BindCol(3, playerClass);
	dbBind.BindCol(4, exp);
	dbBind.BindCol(5, gold);
	dbBind.BindCol(6, hp);
	dbBind.BindCol(7, itemInstanceId);
	dbBind.BindCol(8, itemTemplateId);
	dbBind.BindCol(9, slotIndex);
	dbBind.BindCol(10, itemCount);

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
				player->SetPlayerInfo(playerId, static_cast<Protocol::ClassType>(playerClass), exp, gold, hp);
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

	std::string szNickname = Utf16ToUtf8(nickname);

	Protocol::S_LoginSuccessPacket loginPkt;

	if (loginSuccess)
	{
		std::cout << " Login Success!Inventory Item Count : " << itemPkt.playeritems_size() << std::endl;

		GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
		PlayerRef player = gameSession->_player;
		player->SetName(szNickname);

		Protocol::PlayerInfo* playerInfo = loginPkt.mutable_playerinfo();
		playerInfo->set_playerclass(static_cast<Protocol::ClassType>(player->_class));
		playerInfo->set_playerid(player->_playerId);
		playerInfo->set_playernickname(player->GetName());
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
	if (!gameSession)return false;
	PlayerRef player = gameSession->_player;
	if (!player) return false;

	// 세션 스레드(네트워크 스레드)에서 Redis를 미리 조회하여 Room 스레드 블로킹을 방지합니다.
	int32 ttl = GRedisManager.GetDeathPenaltyTtl(player->GetName());
	if (ttl > 0)
	{
		player->_deathPenaltyTtl = ttl;
	}

	GRoom->DoAsync(&Room::HandleEnterPlayer, player);

	return true;
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

bool PacketHandler::HandleDash(PacketSessionRef& session, Protocol::C_DashPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	if (player == nullptr)
		return false;

	GRoom->DoAsync(&Room::HandlePlayerDash, pkt, player);
	return true;
}

bool PacketHandler::HandleJump(PacketSessionRef& session, Protocol::C_JumpPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	if (player == nullptr)
		return false;

	GRoom->DoAsync(&Room::HandlePlayerJump, pkt, player);
	return true;
}

bool PacketHandler::HandleAttack(PacketSessionRef& session, Protocol::C_AttackPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	if (player == nullptr)
		return false;

	GRoom->DoAsync(&Room::HandleAttack, pkt, player);
	return true;
}


bool PacketHandler::HandleChangeHp(PacketSessionRef& session, Protocol::C_ChangeHpPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	if (player == nullptr)
		return false;

	int32 hp = pkt.hp();
	std::string name = player->GetName();
	player->SetHp(hp);
	GRedisManager.UpdatePlayerHp(name, hp);
	std::cout << "Player " << player->GetName() << " HP Changed: " << player->GetHp() << " (Redis updated)" << std::endl;

	return false;
}

bool PacketHandler::HandleDedicated(PacketSessionRef& session, Protocol::C_DedicatedPacket& pkt)
{
	DedicatedSessionRef dediSession = static_pointer_cast<DedicatedSession>(session);
	dediSession->SetAddrInfo(pkt.serverip(), pkt.serverport());

	std::cout << "Dedicated Server Registered: "
		<< pkt.serverip() << ":" << pkt.serverport() << std::endl;

	return true;
}

bool PacketHandler::HandleDungeonWaitingRoom(PacketSessionRef& session, Protocol::C_DungeonWaitingRoomEnterPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleWaitingRoom, player);
	return true;
}

bool PacketHandler::HandleDungeonCreate(PacketSessionRef& session, Protocol::C_DungeonCreatePacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleCreateDungeon, player);
	return true;
}

bool PacketHandler::HandleDungeonEnter(PacketSessionRef& session, Protocol::C_DungeonEnterPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	int32 dungeonId = pkt.dungeonid();
	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleEnterDungeon, player, dungeonId);
	return true;
}

bool PacketHandler::HandleDungeonExit(PacketSessionRef& session, Protocol::C_DungeonExitPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	int32 dungeonId = pkt.dungeonid();
	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleExitPacket, player, dungeonId);
	return false;
}

bool PacketHandler::HandleDungeonReady(PacketSessionRef& session, Protocol::C_DungeonReadyPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	int32 dungeonId = pkt.dungeonid();

	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleReadyPacket, player, dungeonId);
	return true;
}

bool PacketHandler::HandleDungeonStart(PacketSessionRef& session, Protocol::C_DungeonStartPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	int32 dungeonId = pkt.dungeonid();
	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleDungeonStart, player, dungeonId);
	return true;
}

bool PacketHandler::HandleDungeonEnd(PacketSessionRef& session, Protocol::C_RequestDungeonCompletePacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;
	int32 dungeonId = player->GetDungeonId();

	int32 playerId = pkt.playerid();
	int32 gold = pkt.gold();

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind<2, 1> dbBind(*dbConnect, L"{CALL sp_SetDungeonReward(?, ?)");
	
	dbBind.BindCol(0, gold);
	dbBind.BindCol(1, playerId);

	int resultCode = 0;

	dbBind.BindParam(0, resultCode);

	if (!dbBind.Execute())
	{
		std::cout << "HandleDungeonEnd - Execute() Error" << std::endl;
	}
	if (!dbBind.Fetch())
	{
		std::cout << "HandleDungeonEnd - Fetch() Error" << std::endl;
	}

	if (!resultCode) return false;

	GDungeonWaitingRoom->DoAsync(&DungeonWaitingRoom::HandleDungeonEnd, dungeonId);
	player->SetDungeonId(0);
	Protocol::S_RequestDungeonCompletePacket endPacket;
	endPacket.set_gold(gold);
	SendBufferRef endBuffer = PacketHandler::MakeSendBuffer(endPacket);
	return false;
}

bool PacketHandler::HandleStorePurchase(PacketSessionRef& session, Protocol::C_StorePurchasePacket& pkt)
{
	DBConnection* dbConnect = GDBConnectionPool->Pop();

	// 플레이어 아이디, 아이템 아이디 넘기고 잔액을 받음
	DBBind<2, 6> dbBind(*dbConnect, L"{CALL sp_PurchaseItem(?, ?)}");

	int32 characterId = pkt.playerid();
	int32 itemId = pkt.itemid();

	dbBind.BindParam(0, characterId);
	dbBind.BindParam(1, itemId);

	int32 errorCode = -1;
	int32 remainingGold = 0;
	int32 itemInstanceId = 0;
	int32 itemTemplateId = 0;
	int32 SlotIndex = 0;
	int32 count = 0;

	std::wcout.imbue(std::locale("kor"));

	dbBind.BindCol(0, errorCode);
	dbBind.BindCol(1, remainingGold);
	dbBind.BindCol(2, itemInstanceId);
	dbBind.BindCol(3, itemTemplateId);
	dbBind.BindCol(4, SlotIndex);
	dbBind.BindCol(5, count);

	if (dbBind.Execute())
	{
		if (dbBind.Fetch())
		{
			std::cout << "ResultCode : " << errorCode << std::endl;
		}
	}
	else
	{
		return false;
	}
	GDBConnectionPool->Push(dbConnect);

	Protocol::S_StorePurchasePacket purchasePacket;
	Protocol::ItemData* item = purchasePacket.mutable_iteminfo();
	item->set_iteminstancedid(itemInstanceId);
	item->set_itemtemplateid(itemTemplateId);
	item->set_slotindex(SlotIndex);
	item->set_count(count);

	purchasePacket.set_gold(remainingGold);

	SendBufferRef purchaseBuffer = PacketHandler::MakeSendBuffer(purchasePacket);
	session->Send(purchaseBuffer);

	return true;
}

bool PacketHandler::HandleUseItem(PacketSessionRef& session, Protocol::C_UseItemPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind<2, 5> dbBind(*dbConnect, L"{CALL sp_UseItem(?, ?)}");
	int32 characterId = pkt.playerid();
	int32 slot = pkt.slotindex();

	dbBind.BindParam(0, characterId);
	dbBind.BindParam(1, slot);

	int32 errorCode = -1;
	int32 slotIndex = -1;
	int32 itemCount = -1;
	WCHAR effectType[51] = { 0, };
	int32 effectValue = 0;

	std::wcout.imbue(std::locale("kor"));

	dbBind.BindCol(0, errorCode);
	dbBind.BindCol(1, slotIndex);
	dbBind.BindCol(2, itemCount);
	dbBind.BindCol(3, effectType);
	dbBind.BindCol(4, effectValue);

	if (dbBind.Execute())
	{
		if (dbBind.Fetch())
		{
			std::cout << "ResultCode : " << errorCode << std::endl;
		}
	}
	else
	{
		return false;
	}

	// TODO 아이템 실패 패킷 보내기
	if (errorCode == -1)
	{
		GDBConnectionPool->Push(dbConnect);
		return false;
	}

	std::string szEffectType = Utf16ToUtf8(effectType);
	GDBConnectionPool->Push(dbConnect);

	Protocol::S_UseItemPacket useItemPacket;
	useItemPacket.set_slotindex(slotIndex);
	useItemPacket.set_count(itemCount);
	useItemPacket.set_effecttype(szEffectType);
	useItemPacket.set_effectvalue(effectValue);

	SendBufferRef useItemBuffer = PacketHandler::MakeSendBuffer(useItemPacket);
	session->Send(useItemBuffer);
	return true;
}

bool PacketHandler::HandleChat(PacketSessionRef& session, Protocol::C_ChatPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	std::cout << "Chat PlayerId(" << pkt.playerid() << "): " << pkt.chat() << std::endl;
	Protocol::S_ChatPacket chatPacket;
	chatPacket.set_playerid(player->GetName());
	chatPacket.set_chat(pkt.chat());
	GRoom->DoAsync(&Room::HandleChat, chatPacket);
	return true;
}

bool PacketHandler::HandleMailSend(PacketSessionRef& session, Protocol::C_SendMailPacket& pkt)
{
	DBConnection* dbConnect = GDBConnectionPool->Pop();

	DBBind<8, 0> dbBind(*dbConnect, L"{? = CALL sp_SendMail(?, ?, ?, ?, ?, ?, ?)}");

	int32 returnValue = 0;
	int32 senderId = static_cast<int32>(pkt.senderid());
	int32 gold = pkt.gold();
	int32 itemId = pkt.itemid();
	int32 itemCount = pkt.itemcount();

	std::wstring wReceiverName = Utf8ToUtf16(pkt.receivername());
	std::wstring wTitle = Utf8ToUtf16(pkt.title());
	std::wstring wContent = Utf8ToUtf16(pkt.content());

	dbBind.BindParam(0, returnValue, SQL_PARAM_OUTPUT);
	dbBind.BindParam(1, senderId);
	dbBind.BindParam(2, wReceiverName.c_str());
	dbBind.BindParam(3, wTitle.c_str());
	dbBind.BindParam(4, wContent.c_str());
	dbBind.BindParam(5, gold);
	dbBind.BindParam(6, itemId);
	dbBind.BindParam(7, itemCount);

	bool isSuccess = false;
	Protocol::MailFailReason reason = Protocol::MailFailReason::NONE_ERROR;
	if (dbBind.Execute())
	{
	}
	else
	{
		isSuccess = (returnValue == 0);
		if (!isSuccess)
		{
			if (returnValue == -2)
			{
				reason = Protocol::MailFailReason::NONE_RECEIVER;
			}
			else
			{
				reason = Protocol::MailFailReason::NONE_GOLD;
			}
		}
	}

	GDBConnectionPool->Push(dbConnect);

	Protocol::S_MailSendPacket resultPkt;
	resultPkt.set_success(isSuccess);
	resultPkt.set_errorcode(reason);

	SendBufferRef sendBuffer = PacketHandler::MakeSendBuffer(resultPkt);
	session->Send(sendBuffer);

	return true;
}

bool PacketHandler::HandleMailList(PacketSessionRef& session, Protocol::C_MailListPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind <1, 6> dbBind(*dbConnect, L"{CALL sp_GetMail(?)}");

	int32 playerId = player->GetId();
	dbBind.BindParam(0, playerId);

	int32 mailId = 0;
	WCHAR senderName[51] = { 0, };
	WCHAR title[51] = { 0, };
	int8 isReceived = 0;
	int8 isRead = 0;
	WCHAR expiredDate[51] = { 0, };

	std::wcout.imbue(std::locale("kor"));

	dbBind.BindCol(0, mailId);
	dbBind.BindCol(1, senderName);
	dbBind.BindCol(2, title);
	dbBind.BindCol(3, isReceived);
	dbBind.BindCol(4, isRead);
	dbBind.BindCol(5, expiredDate);

	Protocol::S_MailListPacket listPacket;
	if (dbBind.Execute())
	{
		while (dbBind.Fetch())
		{
			Protocol::MailListInfo* mailInfos = listPacket.add_maillists();

			std::string szTitle = Utf16ToUtf8(title);
			std::string szSender = Utf16ToUtf8(senderName);
			std::string szExpire = Utf16ToUtf8(expiredDate);

			bool hasItem = (isReceived != 0);

			mailInfos->set_mailid(mailId);
			mailInfos->set_sendername(szSender);
			mailInfos->set_title(szTitle);
			mailInfos->set_expireddate(szExpire);
			mailInfos->set_hasitem(hasItem);
		}
	}

	GDBConnectionPool->Push(dbConnect);

	SendBufferRef mailListBuffer = PacketHandler::MakeSendBuffer(listPacket);
	session->Send(mailListBuffer);
	return true;
}

bool PacketHandler::HandleMailContent(PacketSessionRef& session, Protocol::C_MailContentPacket& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	PlayerRef player = gameSession->_player;

	DBConnection* dbConnect = GDBConnectionPool->Pop();
	DBBind <1, 11> dbBind(*dbConnect, L"{CALL sp_GetMailDetail(?)}");

	int32 inMailId = pkt.mailid();
	dbBind.BindParam(0, inMailId);

	int32 mailId = 0;
	WCHAR senderName[51] = { 0, };
	WCHAR title[51] = { 0, };
	WCHAR content[501] = { 0, };
	int32 gold = 0;
	int32 item = -1;
	int32 itemCount = 0;
	int8 isReceived = 0;
	int8 isRead = 0;
	WCHAR sendDate[51] = { 0, };
	WCHAR expiredDate[51] = { 0, };

	std::wcout.imbue(std::locale("kor"));

	dbBind.BindCol(0, mailId);
	dbBind.BindCol(1, senderName);
	dbBind.BindCol(2, title);
	dbBind.BindCol(3, content);
	dbBind.BindCol(4, gold);
	dbBind.BindCol(5, item);
	dbBind.BindCol(6, itemCount);
	dbBind.BindCol(7, isReceived);
	dbBind.BindCol(8, isRead);
	dbBind.BindCol(9, sendDate);
	dbBind.BindCol(10, expiredDate);

	Protocol::S_MailContentPacket contentPacket;

	if (dbBind.Execute())
	{
		if (dbBind.Fetch())
		{
			std::string szSender = Utf16ToUtf8(senderName);
			std::string szTitle = Utf16ToUtf8(title);
			std::string szContent = Utf16ToUtf8(content);
			std::string szSendDate = Utf16ToUtf8(sendDate);
			std::string szExpire = Utf16ToUtf8(expiredDate);

			contentPacket.set_mailid(mailId);
			contentPacket.set_sendername(szSender);
			contentPacket.set_title(szTitle);
			contentPacket.set_content(szContent);
			contentPacket.set_gold(gold);
			contentPacket.set_itemid(item);
			contentPacket.set_itemcount(itemCount);
			contentPacket.set_isreceived(isReceived != 0);
			contentPacket.set_isread(isRead != 0);
			contentPacket.set_senddate(szSendDate);
			contentPacket.set_expireddate(szExpire);
		}
	}

	GDBConnectionPool->Push(dbConnect);

	SendBufferRef mailContentBuffer = PacketHandler::MakeSendBuffer(contentPacket);
	session->Send(mailContentBuffer);

	return true;
}

