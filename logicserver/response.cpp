#include "response.h"
#include "playersession.h"
#include <model/fsprotocol.h>
#include <model/characterinfo.h>
#include <model/iteminfo.h>
#include <model/tpl/templateinfo.h>
#include <model/tpl/item.h>
#include <model/taskinfo.h>
#include <model/mailinfo.h>
#include <model/spellinfo.h>
#include <model/property.h>
#include <model/cdinfo.h>
#include <model/buffinfo.h>
#include <model/friendinfo.h>
#include <model/polycopperinfo.h>
#include <model/tpl/templateloader.h>
#include <model/profile/public.h>
#include <base/gateway/packet.h>
#include <base/gateway/userclient.h>

namespace fs
{
    using namespace std;
    using namespace base;
    using namespace base::gateway;
    using namespace model;

    ///helper ostream function

#define MAKE_PKTOUT(code , approx_size) PacketOut pktout((uint16_t)code , approx_size , ps_.client()->mempool())
#define SEND(pktout) pktout.SetSessionID(session_); ps_.client()->Send(pktout)
#define SEND_SINGLE(pktout) ps_.client()->Send(pktout)

    ///Response

    Response::Response ( PlayerSession& ps )
        :ps_ ( ps ) , session_ ( 0 )
    {
    }

    void Response::SendLoginResponse ( bool result )
    {
        MAKE_PKTOUT ( LOGIN_RESPONSE,30 );
        pktout << result;
        SEND ( pktout );
    }

    void Response::SendPing ( uint16_t sequence )
    {
        MAKE_PKTOUT ( PING,30 );
        pktout << sequence;
        SEND ( pktout );
    }

    void Response::SendPingResult ( uint16_t delay )
    {
        MAKE_PKTOUT ( PING_RESULT,30 );
        pktout << delay;
        SEND ( pktout );
    }

    void Response::SendNotice ( const char* msg, tpl::NoticeType type )
    {
        MAKE_PKTOUT ( NOTICE , 60 );
        pktout << ( uint8_t ) type;
        pktout << msg;
        SEND ( pktout );
    }

    void Response::SendLogout ( uint8_t reason )
    {
        MAKE_PKTOUT ( LOGOUT,30 );
        pktout << reason;
        SEND ( pktout );
    }

    void Response::SendExpUpdate ( uint32_t exp )
    {
        MAKE_PKTOUT ( EXP_UPDATE , 30 );
        pktout << exp;
        SEND ( pktout );
    }

    void Response::SendEnergyUpdate ( uint16_t energy )
    {
        MAKE_PKTOUT ( ENERGY_UPDATE , 30 );
        pktout << energy;
        SEND ( pktout );
    }

    void Response::SendCharacterLevelUp ( uint16_t level )
    {
        MAKE_PKTOUT ( LEVEL_UP , 30 );
        pktout << level;
        SEND ( pktout );
    }

    void Response::SendCharacterUpdate ( const CharacterInfo& info )
    {
        MAKE_PKTOUT ( UPDATE_CHARACTER_INFO , 500 );
        //cout<<info.uid() <<" "<<info.nickname() << ( uint8_t ) info.table().profession<<endl;
        pktout << info.uid() << info.nickname() ;
        pktout << info.table().level<< info.table().exp << info.table().gold << info.table().money << info.table().bind_gold << info.table().bind_money;
        pktout << info.table().energy << ( uint8_t ) info.table().profession;
        SEND ( pktout );
    }

    void Response::SendPlayerSpeedUpdate ( uint16_t speed )
    {
        MAKE_PKTOUT ( UPDATE_PLAYER_SPEED , 24 );
        pktout << speed;
        SEND ( pktout );
    }
    void Response::SendNickNameUpdate ( uint16_t result )
    {
        MAKE_PKTOUT ( UPDATE_NICKNAME_RESPONSE , 24 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }


    void Response::SendBagUpdate ( const std::vector< const ItemInfo* >& items, const vector< uint16_t >& removelist )
    {
        MAKE_PKTOUT ( BAG_UPDATE , 120 );
        pktout.WriteUShort ( items.size() );
        for ( size_t i = 0; i < items.size(); ++i ) {
            const ItemInfo* item = items[i];
            pktout.WriteUShort ( item->table().pos );
            pktout.WriteULong ( item->table().itemid );
            pktout.WriteInt ( item->table().tpid );
            pktout.WriteByte ( ( uint8_t ) item->table().bind );
            pktout.WriteByte ( ( uint8_t ) item->tpl().type );
            if ( item->tpl().type == tpl::iEquip ) {
                const model::EquipInfo* equip = item->ToEquipInfo();
                pktout.WriteUShort ( equip->strength_level() );
                pktout << equip->attrs();
                pktout << equip->gemslots();
            } else if ( item->tpl().type == tpl::iProp ) {
                const model::PropInfo* prop = item->ToPropInfo();
                pktout.WriteUShort ( prop->stack_count() );
            }
        }
        pktout << removelist;
        SEND ( pktout );
    }

    void Response::SendBagUpdateOpenCount ( uint16_t count )
    {
        MAKE_PKTOUT ( UPDATE_BAG_OPEN_COUNT , 16 );
        pktout << count;
        SEND ( pktout );
    }

    void Response::SendBagGridUnlock ( uint8_t result )
    {
        MAKE_PKTOUT ( BAG_GRID_UNLOCK_RESPONSE , 16 );
        pktout << result;
        SEND ( pktout );
    }

    void Response::SendPropertyUpdate ( const Property& property )
    {
        MAKE_PKTOUT ( PROPERTY_UPDATE , 64 );
        pktout << property;
        SEND ( pktout );
    }

    void Response::SendSpellUpdate ( const SpellInfo& spell, const vector< int32_t >& pendings )
    {
        MAKE_PKTOUT ( SPELL_UPDATE , 120 );
        pktout.WriteUShort ( spell.tree().size() );
        for ( model::spell_tree_t::const_iterator it= spell.tree().begin() ; it != spell.tree().end(); ++it ) {
            pktout << it->second.tpl->spell_id << it->second.level;
        }
        pktout << pendings;
        SEND ( pktout );
    }

    void Response::SendSpellBarUpdate ( const SpellInfo& spell )
    {
        MAKE_PKTOUT ( SPELLBAR_UPDATE , 60 );
        const model::spell_bar_t& bar = spell.spellbar();
        pktout.WriteUShort ( bar.size() );
        for ( size_t i = 0 ; i < bar.size(); ++i ) {
            if ( bar[i] == nullptr ) {
                pktout.WriteInt ( 0 );
            } else {
                pktout.WriteInt ( bar[i]->tpl->spell_id );
            }
        }
        SEND ( pktout );
    }

    void Response::SendCDListUpdate ( const std::vector< const CDInfo* > updates )
    {
        MAKE_PKTOUT ( CDLIST_UPDATE , 40 );
        pktout.WriteUShort ( updates.size() );
        for ( vector< const CDInfo* >::const_iterator it = updates.begin(); it != updates.end(); ++it ) {
            const CDInfo* cd = *it;
            pktout << ( uint8_t ) cd->type() << cd->GetCooldownTime() << cd->current() << cd->max();
        }
        SEND ( pktout );
    }

    void Response::SendBuffUpdate ( const std::vector< const BuffInfo* >& updates, const vector< int32_t >& removelist )
    {
        MAKE_PKTOUT ( BUFF_UPDATE , 40 );
        pktout.WriteUShort ( updates.size() );
        for ( vector< const BuffInfo* >::const_iterator it = updates.begin(); it != updates.end(); ++it ) {
            const BuffInfo* buff = *it;
            pktout << buff->tplid() << buff->GetExpireTime();
        }
        pktout << removelist;
        SEND ( pktout );
    }

    void Response::SendWorkshopStrength ( bool result )
    {
        MAKE_PKTOUT ( WORKSHOP_STRENGTH_RESPONSE , 16 );
        pktout << result;
        SEND ( pktout );
    }

    void Response::SendWalletUpdate ( const CharacterInfo& info )
    {
        MAKE_PKTOUT ( WALLET_UPDATE , 32 );
        pktout << info.table().gold << info.table().money
               << info.table().bind_gold << info.table().bind_money;
        SEND ( pktout );
    }

    void Response::SendWorkshopStrengthReplace ( bool result )
    {
        MAKE_PKTOUT ( WORKSHOP_STRENGTH_REPLACE_RESPONSE , 16 );
        pktout << result;
        SEND ( pktout );
    }

    void Response::SendWorkshopGemRemove ( uint64_t gem_itemid )
    {
        MAKE_PKTOUT ( WORKSHOP_GEM_REMOVE_RESPONSE , 16 );
        pktout << gem_itemid;
        SEND ( pktout );
    }

    void Response::SendWorkshopMake ( bool result )
    {
        MAKE_PKTOUT ( WORKSHOP_MAKE_RESPONSE , 16 );
        pktout << result;
        SEND ( pktout );
    }

    void Response::SendWorkshopWashResultUpdate ( uint64_t itemid, const vector< tpl::RandomAttribute >& attrs )
    {
        MAKE_PKTOUT ( WORKSHOP_WASH_RESULT_UPDATE , 16 );
        pktout << itemid;
        pktout << attrs;
        SEND ( pktout );
    }

    void Response::SendWorkshopInlay ( bool result )
    {
        MAKE_PKTOUT ( WORKSHOP_INLAY_RESPONSE , 16 );
        pktout << result;
        SEND ( pktout );
    }

    void Response::PrivateChat ( uint32_t uid ,string nickname ,string msg )
    {
        MAKE_PKTOUT ( CHAT_SEND_RESPONSE , 100 );
        pktout.WriteUInt ( uid );
        pktout.WriteString ( nickname );
        pktout.WriteString ( msg );
        pktout.WriteByte ( model::tpl::cPrivate );
        cout<<" Response::PrivateChat : "<<uid<<"  "<<nickname << "  "<<msg<<endl;
        SEND ( pktout );
    }
    void Response::WorldChat ( uint32_t uid, string nickname, string msg, uint32_t level, model::tpl::ProfessionType profession )
    {
        MAKE_PKTOUT ( CHAT_SEND_RESPONSE , 100 );
        pktout << uid;
        pktout << nickname;
        pktout << msg;
        pktout.WriteByte ( model::tpl::cWorld );
        pktout<< level;
        pktout<< ( uint32_t ) profession;
        SEND ( pktout );
    }

    void Response::SendMailRrivateResponese ( bool result,int8_t msg )
    {
        MAKE_PKTOUT ( MAIL_PRIVATE_RESPONSE , 30 );
        pktout.WriteBoolean(result);
        if(msg!=0){
            pktout.WriteBoolean(msg);
        }
        SEND ( pktout );
    }


    void Response::SendMailCount ( uint32_t count )
    {
        MAKE_PKTOUT ( MAIL_UNREAD_RESPONSE , 30 );
        pktout << count;
        SEND ( pktout );
    }

    void Response::SendMailList ( int32_t& begin,int32_t& end,  std::vector<model::MailInfo>& mlist ,int32_t size,int8_t box_type )
    {
        MAKE_PKTOUT ( MAIL_LIST_RESPONSE , 1024 );
        pktout.WriteUShort ( end-begin );
        for ( int32_t i=begin; i<end; i++ ) {
            pktout.WriteUInt ( mlist[i].id() );
            pktout.WriteUInt ( mlist[i].send_player_id() );
            if ( box_type==1 ) {
                if ( mlist[i].send_player_name().empty() )
                    pktout.WriteString ( "" );
                else {
                    pktout.WriteString ( mlist[i].send_player_name() );
                }
            }
            else {
                if ( mlist[i].recv_player_name().empty() )
                    pktout.WriteString ( "" );
                else {
                    pktout.WriteString ( mlist[i].recv_player_name() );
                }
            }
            pktout.WriteString ( mlist[i].title() );
            cout<<"id\t"<< mlist[i].id() <<" send_player_id:"<< mlist[i].send_player_id() <<" ";
            //0: 发件箱。1: 收件箱
            int64_t time;
            //if ( box_type== ( int8_t ) 1 ) {
            time = mlist[i].send_time();
            //} else {
            //    time = mlist[i].read_time();
            //}
            if ( mlist[i].type() == model::tpl::mMailSystem ) {
                pktout.WriteLong ( time );
                cout<<"mMailSystem mail id\t"<<mlist[i].id() <<"\t"<<time <<endl;
            } else {
                pktout.WriteLong ( time );
                cout<<"pri mail id\t"<<mlist[i].id() <<"\t"<<time <<endl;
            }
            pktout.WriteBoolean ( mlist[i].is_read() );
        }
        //总页数
        ushort pagesize = mlist.size() / size +1;
        pktout.WriteUShort ( pagesize );
        pktout.WriteByte ( box_type );
        SEND ( pktout );
    }

    void Response::ReadMail ( model::MailInfo& info ,vector<const model::ItemInfo*>& item )
    {
        MAKE_PKTOUT ( READ_MAIL_RESPONSE , 30 );
        //pktout << info.title() ;
        pktout << info.content() ;
        cout<<"info.type()  :"<< ( uint32_t ) info.type() <<endl;
        pktout.WriteByte ( info.type() );
        if ( info.type() == model::tpl::mMailSystem ) {
            pktout.WriteString ( ( info.atta_explain().empty() || info.atta_explain() =="" ) ? "": info.atta_explain() );
            ushort itemsize =item.size();
            pktout.WriteUShort ( itemsize );
            for ( uint32_t item_i=0; item_i<itemsize; item_i++ ) {
                pktout.WriteUShort ( item[item_i]->table().pos );
                pktout.WriteULong ( item[item_i]->table().itemid );
                pktout.WriteInt ( item[item_i]->table().tpid );
                pktout.WriteBoolean ( ( uint8_t ) item[item_i]->table().bind );
                pktout.WriteByte ( ( uint8_t ) item[item_i]->tpl().type );
                if ( item[item_i]->tpl().type == tpl::iEquip ) {
                    const model::EquipInfo* equip = item[item_i]->ToEquipInfo();
                    pktout.WriteUShort ( equip->strength_level() );
                    pktout << equip->attrs();
                    pktout << equip->gemslots();
                } else if ( item[item_i]->tpl().type == tpl::iProp ) {
                    const model::PropInfo* prop = item[item_i]->ToPropInfo();
                    pktout.WriteUShort ( prop->stack_count() );
                }
            }
        }
        SEND ( pktout );
    }

    void Response::SendMailDelResult ( vector< uint32_t >& fresult )
    {
        MAKE_PKTOUT ( MAIL_DEL_RESPONSE , 30 );
        pktout.WriteUInt ( fresult.size() );
        for ( uint32_t i=0; i<fresult.size(); i++ ) {
            pktout.WriteUInt ( fresult[i] );
        }
        SEND ( pktout );
    }
    void Response::SendMailDelResult ( std::map<uint32_t, int8_t>& fresult )
    {
        MAKE_PKTOUT ( MAIL_DEL_RESPONSE , 30 );
        pktout.WriteUInt ( fresult.size() );
        std::map<uint32_t, int8_t>::iterator itor= fresult.begin();
        for ( ; itor!= fresult.end(); itor ++ ) {
            cout << ( *itor ).first <<"  "<< ( uint32_t ) ( *itor ).second<<endl;
            pktout.WriteUInt ( ( *itor ).first );
            pktout.WriteByte ( ( *itor ).second );
        }
        SEND ( pktout );
    }


    void Response::SendMailDrawAttachmentResponse ( uint32_t eid, uint32_t type )
    {
        MAKE_PKTOUT ( MAIL_ATTACHMENT_RESPONSE , 30 );
        pktout.WriteUInt ( type );
        SEND ( pktout );
    }

    //好友
    void Response::SendFriendGroudAddResponse ( uint32_t result )
    {
        MAKE_PKTOUT ( GROUP_ADD_RESPONSE , 30 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }
    void Response::SendFriendGroudDelResponse ( std::vector< uint8_t > fail_gids )
    {
        MAKE_PKTOUT ( GROUP_DEL_RESPONSE , 30 );
        pktout << fail_gids ;
        SEND ( pktout );
    }
    void Response::SendFriendGroudUpdateResponse ( uint32_t result )
    {
        MAKE_PKTOUT ( GROUP_UPDATE_RESPONSE , 30 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }

    void Response::SendFriendAddResponse ( uint32_t result )
    {
        MAKE_PKTOUT ( FRIENDS_ADD_RESPONSE , 30 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }

    void Response::SendFriendCallAddResponse ( uint32_t uid, const std::string& nickname )
    {
        MAKE_PKTOUT ( FRIENDS_CALLADD_RESPONSE , 30 );
        pktout.WriteUInt ( uid );
        pktout.WriteString ( nickname );
        SEND ( pktout );
    }

    void Response::SendFriendUpdateResponse ( uint32_t result )
    {
        MAKE_PKTOUT ( FRIENDS_UPDATE_RESPONSE , 30 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }
    void Response::SendFriendDeleteMsg ( uint32_t uid )
    {
        MAKE_PKTOUT ( FRIENDS_DEL_MSG_RESPONSE , 30 );
        pktout.WriteUInt ( uid );
        SEND ( pktout );
    }




    void Response::SendFriendList ( std::vector<model::GuildFriend>& friends )
    {
        MAKE_PKTOUT ( FRIENDS_LIST_RESPONSE , 30 );
        pktout.WriteUShort ( friends.size() );
        std::vector<model::GuildFriend>::iterator  temp_iter;
        for ( temp_iter = friends.begin() ; temp_iter != friends.end() ; temp_iter++ ) {
            model::GuildFriend&  gfriend = *temp_iter;
            pktout.WriteByte ( gfriend.gid );
            pktout.WriteString ( gfriend.gname );
            // online
            pktout.WriteUShort ( gfriend.online.size() );
            for ( size_t online_i=0; online_i<gfriend.online.size(); online_i++ ) {
                pktout.WriteUInt ( gfriend.online[online_i]->friend_uid() );
                pktout.WriteByte ( gfriend.online[online_i]->profession() );
                pktout.WriteUShort ( gfriend.online[online_i]->level() );
                pktout.WriteString ( gfriend.online[online_i]->nickname() );
                pktout.WriteUInt ( gfriend.online[online_i]->score() );
            }
            //offline
            pktout.WriteUShort ( gfriend.offline.size() );
            for ( size_t offline_i=0; offline_i<gfriend.offline.size(); offline_i++ ) {
                pktout.WriteUInt ( gfriend.offline[offline_i]->friend_uid() );
                pktout.WriteByte ( gfriend.offline[offline_i]->profession() );
                pktout.WriteUShort ( gfriend.offline[offline_i]->level() );
                cout<<gfriend.offline[offline_i]->nickname() <<endl;
                pktout.WriteString ( gfriend.offline[offline_i]->nickname() );
                pktout.WriteUInt ( gfriend.offline[offline_i]->score() );
            }
        }
        SEND ( pktout );
    }

    void Response::SendFriendApplyPage ( ushort& begin, ushort& end, std::vector< FriendInfo* >& flist ,ushort size )
    {
        MAKE_PKTOUT ( FRIENDS_APPLY_LIST_RESPONSE , 30 );
        pktout.WriteUShort ( end-begin );
        for ( int32_t i=begin; i<end; i++ ) {
            pktout.WriteUInt ( flist[i]->friend_uid() );
            pktout.WriteByte ( flist[i]->profession() );
            pktout.WriteUShort ( flist[i]->level() );
            pktout.WriteString ( flist[i]->nickname() );
        }
        pktout.WriteUShort ( flist.size() /size+1 );
        SEND ( pktout );
    }


    void Response::SendFriendApplyResultResponse ( uint32_t result, uint32_t uid, const string& nickname, uint8_t p, uint16_t level )
    {
        MAKE_PKTOUT ( FRIENDS_ADD_ACTION_RESPONSE , 30 );
        pktout.WriteByte ( result );
        if ( result==1 ) {
            pktout.WriteUInt ( uid );
            pktout.WriteString ( nickname );
            pktout.WriteByte ( p );
            pktout.WriteUShort ( level );
        }
        SEND ( pktout );
    }


    void Response::SendOnlineNotice ( uint32_t uid )
    {
        MAKE_PKTOUT ( FRIENDS_ONLINE_RESPONSE , 30 );
        pktout.WriteUInt ( uid );
        SEND ( pktout );
    }
    void Response::SendMoveFriendGroudNotice ( uint8_t result )
    {
        MAKE_PKTOUT ( FRIENDS_MOVE_RESPONSE , 30 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }
    void Response::SendFrindSettingNotice ( uint8_t result )
    {
        MAKE_PKTOUT ( FRIENDS_REPLY_RESPONSE , 30 );
        pktout.WriteByte ( result );
        SEND ( pktout );
    }
    void Response::SendFriendOneKeySelect ( std::vector<const model::profile::Public*> players )
    {
        MAKE_PKTOUT ( FRIENDS_ONEKEY_SELECT_RESPONSE , 30 );
        pktout.WriteUShort ( players.size() );
        for ( std::vector<const model::profile::Public*>::iterator temp_iter = players.begin() ; temp_iter != players.end() ; temp_iter++ ) {
            pktout.WriteInt ( ( *temp_iter )->base.puid.uid );
            pktout.WriteString ( ( *temp_iter )->base.nickname );
        }
        SEND ( pktout );
    }
    void Response::SendEnmityResponse ( uint32_t uid, const string& nickname, ushort level, ProfessionType p )
    {
        MAKE_PKTOUT ( FRIENDS_ENMITY_RESPONSE , 30 );
        pktout.WriteUInt ( uid );
        pktout.WriteString ( nickname );
        pktout.WriteUShort ( level );
        pktout.WriteByte ( p );
        SEND ( pktout );
    }


    void Response::SendTripodListResponse ( vector< model::PolyCopperRecord > records )
    {
        MAKE_PKTOUT ( TRIPOD_RECODE_RESPONSE , 30 );
        ushort size=records.size();
        pktout.WriteUShort ( size );
        for ( ushort i=0; i<size; i++ ) {
            pktout.WriteByte ( ( int8_t ) records.at ( i ).type() );
            pktout.WriteUInt ( records.at ( i ).uid() );
            pktout.WriteString ( records.at ( i ).nickname() );
            pktout.WriteUInt ( records.at ( i ).exp() );
            pktout.WriteLong ( records.at ( i ).insert_time() );
        }
        SEND ( pktout );
    }
    void Response::SendTripodResponse ( bool cd, uint32_t copper_coin ,uint32_t exp )
    {
        MAKE_PKTOUT ( TRIPOD_RESPONSE , 30 ); //铸鼎
        pktout<<cd<< copper_coin<<exp;
        SEND ( pktout );
    }
    void Response::SendPolyReceive ( uint32_t nextlevel, uint32_t nextgold )
    {
        MAKE_PKTOUT ( RECEIVEGOD_RESPONSE , 30 ); //收取神币
        pktout<<nextlevel<<nextgold;
        SEND ( pktout );
    }

    void Response::SendPolyFriends ( std::vector< FriendInfo* > friends )
    {
        MAKE_PKTOUT ( TRIPOD_FRIEND_LIST_RESPONSE , 30 ); //帮好友铸鼎列表
        pktout.WriteShort ( friends.size() );
        std::vector<model::FriendInfo*>::iterator  temp_iter;
        for ( temp_iter = friends.begin() ; temp_iter != friends.end() ; temp_iter++ ) {
            pktout.WriteUInt ( ( *temp_iter )->friend_uid() );
            pktout.WriteString ( ( *temp_iter )->nickname() );
            pktout.WriteUShort ( ( *temp_iter )->poly_level() );
            pktout.WriteBoolean ( ( *temp_iter )->CanPoly() );
        }
        SEND ( pktout );
    }

    void Response::SendTaskUpdate ( std::vector<model::TaskInfo>& tasks )
    {
        MAKE_PKTOUT ( TASK_UPDATE_RESPONSE , 30 );
        pktout.WriteShort ( tasks.size() );
        std::vector<model::TaskInfo>::iterator  temp_iter;
        for ( temp_iter = tasks.begin() ; temp_iter != tasks.end() ; temp_iter++ ) {
            pktout.WriteUInt ( ( *temp_iter ).taskid() );
            pktout.WriteString ( ( *temp_iter ).taskname() );
            pktout.WriteByte ( ( *temp_iter ).status() );
            pktout.WriteUInt ( ( *temp_iter ).npcid() );
        }
        SEND ( pktout );

    }

}







