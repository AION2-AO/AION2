#include "pch.h"
#include "ObjectUtils.h"
#include "Player.h"
#include "GameSession.h"

std::atomic<uint32> ObjectUtils::_idGenerator = 1;
PlayerRef ObjectUtils::CreatePlayer(GameSessionRef session)
{
    const uint32 newId = _idGenerator.fetch_add(1);

    PlayerRef player = std::make_shared<Player>();
    player->_playerId = newId;
    player->_ownerSession = session;
    session->_player = player;
    return player;

}
