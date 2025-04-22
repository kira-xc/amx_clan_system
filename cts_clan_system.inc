#if defined _cts_clan_system_included
	#endinput
#endif

#define _cts_clan_system_included

#include <amxmodx>
#include <sqlx>
#include <json>


#define CTS_VERSION "0.1_beta"
#define CTS_AUTHOR "CT Spawn"
#define CTS_CLAN_HOMEURL "https://github.com/kira-xc/amx_clan_system"

#define CTS_CLAN_PREFIX "^1[^4Clan System^1]"

new JSON:____cts_clans 

new Handle:____cts_g_SqlTuple



new ____cts_db_users[33],____cts_db_member_in_clan_of[33],____cts_db_is_leader_of[33];
new ____cts_db_names[33][32],____cts_db_steams[33][32];



stock clan_cts_clean_names(name[],len){
    replace_all(name,len,"(1)","")
    replace_all(name,len,"(2)","")
    replace_all(name,len,"(3)","")
    replace_all(name,len,"(4)","")
    replace_all(name,len,"(5)","")
    replace_all(name,len,"(6)","")
    replace_all(name,len,"(7)","")
    replace_all(name,len,"(8)","")
    replace_all(name,len,"(9)","")
    replace_all(name,len,"(10)","")
    replace_all(name,len,"(","")
    replace_all(name,len,")","")
    replace_all(name,len," ","_")
}

/**
 * Initializes the connection to the clan database.
 *
 * @param Host         The database host (e.g., "127.0.0.1").
 * @param User         The database username.
 * @param Pass         The database password.
 * @param Db           The database name.
 */
stock clan_cts_datas_Init(Host[],User[],Pass[],Db[])
{

    ____cts_g_SqlTuple = SQL_MakeDbTuple(Host,User,Pass,Db)
   
    // ok, we're ready to connect
    new ccc_Error[512]
    new ErrorCode,Handle:SqlConnection = SQL_Connect(____cts_g_SqlTuple,ErrorCode,ccc_Error,charsmax(ccc_Error))
    if(SqlConnection == Empty_Handle)
        // stop the plugin with an error message
        set_fail_state(ccc_Error)
       

    // you free everything with SQL_FreeHandle
    SQL_FreeHandle(SqlConnection) 
    ____cts_clans = json_init_object();  
    clan_cts_clan_loads()
}

/**
 * Ends all clan-related plugin functions and performs cleanup.
 */
stock clan_cts_end_plugin()
{
    // free the tuple - note that this does not close the connection,
    // since it wasn't connected in the first place
    SQL_FreeHandle(____cts_g_SqlTuple)

    json_free(____cts_clans);
}

/**
 * Loads clan data for a specific player.
 *
 * @param id           The player's ID.
 */
stock clan_cts_load_player_data(id)
{
    new szSteamId[32],szNamess[32], szTemp[512]
    get_user_authid(id, szSteamId, charsmax(szSteamId))
    get_user_name(id, szNamess, charsmax(szNamess))

    clan_cts_clean_names(szNamess,charsmax(szNamess))
    new Data[1]
    Data[0] = id
    
    //we will now select from the table `tutorial` where the steamid match
    format(szTemp,charsmax(szTemp),"SELECT u.user_id, u.user_name, u.steam_id, cm.clan_id AS member_clan_id, c.id AS leader_clan_id FROM users u LEFT JOIN clan_members cm ON u.user_id = cm.user_id LEFT JOIN clans c ON u.user_id = c.leader_id WHERE u.user_name = '%s' AND u.steam_id = '%s';", szSteamId,szNamess)
    SQL_ThreadQuery(____cts_g_SqlTuple,"______clan_cts_register_client",szTemp,Data,1)
}

public ______clan_cts_register_client(FailState,Handle:Query,Error[],Errcode,Data[],DataSize)
{
    if(FailState != TQUERY_SUCCESS)
    {
        SQL_FreeHandle(Query)
        return ;
    }

    new id
    id = Data[0]
    
    if(SQL_NumResults(Query) < 1) 
    {
        //.if there are no results found
        
        new szSteamId[32],szNamess[32]
        get_user_authid(id, szSteamId, charsmax(szSteamId)) // get user's steamid
        get_user_name(id, szNamess, charsmax(szNamess))
        clan_cts_clean_names(szNamess,charsmax(szNamess))
        //  if its still pending we can't do anything with it
        if (equal(szSteamId,"ID_PENDING")){
            SQL_FreeHandle(Query)
            return ;
        }
            
        new szTemp[512]
        
        // now we will insturt the values into our table.
        format(szTemp,charsmax(szTemp),"INSERT INTO users ( steam_id , user_name)VALUES ('%s','%s');",szSteamId,szNamess)
        SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle",szTemp)
        __clan_cts_load_player_data2(id)
    } 
    else 
    {
        // if there are results found
        ____cts_db_users[id]  = SQL_ReadResult(Query, 0)
        SQL_ReadResult(Query, 1,____cts_db_names[id],31)
        SQL_ReadResult(Query, 2,____cts_db_steams[id],31)
        if(SQL_IsNull(Query,3)){
            ____cts_db_member_in_clan_of[id]=0
        }
        else{
            ____cts_db_member_in_clan_of[id]=SQL_ReadResult(Query, 3)
        }
        if(SQL_IsNull(Query,4)){
            ____cts_db_is_leader_of[id]=0
        }
        else{
            ____cts_db_is_leader_of[id]=SQL_ReadResult(Query, 4)
        }
    }
    SQL_FreeHandle(Query)
    return ;
}

public __clan_cts_load_player_data2(id)
{
    new szSteamId[32],szNamess[32], szTemp[512]
    get_user_authid(id, szSteamId, charsmax(szSteamId))
    get_user_name(id, szNamess, charsmax(szNamess))

    clan_cts_clean_names(szNamess,charsmax(szNamess))
    new Data[1]
    Data[0] = id
    
    //we will now select from the table `tutorial` where the steamid match
    format(szTemp,charsmax(szTemp),"SELECT u.user_id, u.user_name, u.steam_id, cm.clan_id AS member_clan_id, c.id AS leader_clan_id FROM users u LEFT JOIN clan_members cm ON u.user_id = cm.user_id LEFT JOIN clans c ON u.user_id = c.leader_id WHERE u.user_name = '%s' AND u.steam_id = '%s';", szSteamId,szNamess)
    SQL_ThreadQuery(____cts_g_SqlTuple,"______clan_cts_register_client2",szTemp,Data,1)
}

public ______clan_cts_register_client2(FailState,Handle:Query,Error[],Errcode,Data[],DataSize)
{
    if(FailState != TQUERY_SUCCESS)
    {
        SQL_FreeHandle(Query)
        return ;
    }

    new id
    id = Data[0]
    
    if(SQL_NumResults(Query) < 1) 
    {
        //.if there are no results found
        
        new szSteamId[32],szNamess[32]
        get_user_authid(id, szSteamId, charsmax(szSteamId)) // get user's steamid
        get_user_name(id, szNamess, charsmax(szNamess))
        clan_cts_clean_names(szNamess,charsmax(szNamess))
        //  if its still pending we can't do anything with it
        if (equal(szSteamId,"ID_PENDING")){
            SQL_FreeHandle(Query)
            return ;
        }
            
        new szTemp[512]
        
        // now we will insturt the values into our table.
        format(szTemp,charsmax(szTemp),"INSERT INTO users ( steam_id , user_name)VALUES ('%s','%s');",szSteamId,szNamess)
        SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle",szTemp)
    } 
    else 
    {
        // if there are results found
        ____cts_db_users[id]  = SQL_ReadResult(Query, 0)
        SQL_ReadResult(Query, 1,____cts_db_names[id],31)
        SQL_ReadResult(Query, 2,____cts_db_steams[id],31)
        if(SQL_IsNull(Query,3)){
            ____cts_db_member_in_clan_of[id]=0
        }
        else{
            ____cts_db_member_in_clan_of[id]=SQL_ReadResult(Query, 3)
        }
        if(SQL_IsNull(Query,4)){
            ____cts_db_is_leader_of[id]=0
        }
        else{
            ____cts_db_is_leader_of[id]=SQL_ReadResult(Query, 4)
        }
    }
    SQL_FreeHandle(Query)
    return ;
}


/**
 * Creates a new clan with the specified leader, prefix, name, and members.
 *
 * @param id           The ID of the player creating the clan (leader).
 * @param prefix[]     A short clan prefix (e.g., "[CLN]").
 * @param clan_name[]  The full name of the clan.
 * @param memebers[]   An array of user IDs to add as initial members.
 * @param max_member   The maximum number of members allowed in the clan.
 */
stock clan_cts_create_clan(id,prefix[],clan_name[],memebers[32],max_member=0)
{
    new leader_id=____cts_db_users[id]
    new member_of_clan=____cts_db_member_in_clan_of[id]
    if(leader_id==0 || member_of_clan!=0 ){
        return ;
    }

    new Data[1]
    Data[0] = id
    new count=0,szTemp[512]
    for(new i=0;i<max_member;i++)
    {
        if (memebers[i]>0){
            count++;
        }
    }
    if (count<max_member){
        return ;
    }


    // Here we will update the user hes information in the database where the steamid matches.
    format(szTemp,charsmax(szTemp),"INSERT INTO clans (name, clan_prefix,  leader_id) VALUES ( '%s', '%s', %d);",clan_name, prefix,leader_id)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)

    format(szTemp,charsmax(szTemp),"SELECT id,clan_prefix,clan_logo_motd,upgrade_level,DATEDIFF(last_payment+INTERVAL 14 DAY,now() ) AS daysleft,name,leader_id FROM clans where name = '%s';",clan_name)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_register_members",szTemp,memebers,32)

}

public _____clan_cts_register_members(FailState,Handle:Query,Error[],Errcode,memebers[],DataSize)
{
    if(FailState != TQUERY_SUCCESS)
    {
        return ;
    }
    new clanid;

    if(SQL_NumResults(Query) < 1) 
    {
        SQL_FreeHandle(Query)
        return ;
    } 
    else 
    {
        // if there are results found
        clanid  = SQL_ReadResult(Query, 0)
        ____clan_cts_add_clan_in_json(Query)
    }
    
    new count=0,szTemp[512]
    for(new i=0;i<32;i++)
    {
        if (memebers[i]>0){
            for(new j=1;j<33;j++){
                if(____cts_db_users[j]==memebers[i]){
                    if(is_user_connected(j)){
                        ____cts_db_member_in_clan_of[j]=clanid
                        break
                    }
                }
            }
            count++;
        }
    }
    for (new i=0;i<count;i++){
        format(szTemp,charsmax(szTemp),"INSERT INTO clan_members (user_id, clan_id) VALUES (%d, %d);",memebers[i],clanid)
        SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle",szTemp)
    }
    SQL_FreeHandle(Query)
    return ;
}

public _____clan_cts_IgnoreHandle_Edited(FailState,Handle:Query,Error[],Errcode,Data[],DataSize)
{
    new id
    id = Data[0]
    if(FailState != TQUERY_SUCCESS)
    {
        client_print_color(id, print_team_red, "%s ^3error ^1!!",CTS_CLAN_PREFIX)
    }

    SQL_FreeHandle(Query)
    return ;
}

public _____clan_cts_IgnoreHandle(FailState,Handle:Query,Error[],Errcode,Data[],DataSize)
{

    SQL_FreeHandle(Query)
    
    return ;
}

/**
 * Adds a new player to the clan using their player ID.
 *
 * @param id               The ID of the clan leader or admin.
 * @param id_new_member    The ID of the player to be added.
 */
stock clan_cts_add_member_in_clan(id,id_new_member){
    new user_id_new=____cts_db_users[id_new_member],user_id=____cts_db_users[id]
    new member_of_clan=____cts_db_member_in_clan_of[id],member_of_clan_new=____cts_db_member_in_clan_of[id_new_member]
    if(user_id==0 || user_id_new==0 || member_of_clan==0 ){
        return ;
    }
    else if(member_of_clan_new!=0){
        return ;
    }
    new Data[1],szTemp[512]
    Data[0] = id
    format(szTemp,charsmax(szTemp),"INSERT INTO clan_members (user_id, clan_id) VALUES ( '%d', '%d');",user_id_new, member_of_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
    if(is_user_connected(id_new_member)){
        ____cts_db_member_in_clan_of[id_new_member]=member_of_clan
    }
}

/**
 * Adds a new player to the clan using their user ID.
 *
 * @param id                   The ID of the clan leader or admin.
 * @param userid_new_member    The user ID of the player to be added.
 *
 * @note this option for is add from request list and the player will add in next map after
 * use the function
 */
stock clan_cts_add_member_in_clan_by_userid(id,userid_new_member){
    new user_id=____cts_db_users[id]
    new member_of_clan=____cts_db_member_in_clan_of[id]
    if(user_id==0 || userid_new_member==0 || member_of_clan==0 ){
        return ;
    }

    new Data[1],szTemp[512]
    Data[0] = id
    format(szTemp,charsmax(szTemp),"INSERT INTO clan_members (user_id, clan_id) VALUES ( '%d', '%d');",userid_new_member, member_of_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
}


/**
 * Removes a member from the clan, must be called by the leader.
 *
 * @param leader       The ID of the clan leader.
 * @param id_member    The ID of the member to be removed.
 */
stock clan_cts_remove_member_from_clan(leader,id_member){
    new leader_of = ____cts_db_is_leader_of[leader],user_id=____cts_db_users[id_member]
    new member_of_clan=____cts_db_member_in_clan_of[id_member]
    if(leader_of==0 || user_id==0 || member_of_clan==0 ){
        return ;
    }
    else if(leader_of!=member_of_clan){
        return ;
    }


    new Data[1],szTemp[512]
    Data[0] = leader
    format(szTemp,charsmax(szTemp),"delete from clan_members where user_id=%d and clan_id=%d;",user_id, member_of_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
    ____cts_db_member_in_clan_of[id_member]=0
}

/**
 * Allows a player to donate coins to their clan.
 *
 * @param id           The player's ID.
 * @param coins        The amount of coins to donate.
 * @param callbackFunc[] Callback function to be called after donation.
 *
 * @note callback will return callbackFunc(id,balance_coins)
 */
stock clan_cts_donate_clan(id,coins,callbackFunc[]){
    new user_id = ____cts_db_users[id] , member_in_clan = ____cts_db_member_in_clan_of[id]
    if (user_id==0){
        return;
    }
    else if (member_in_clan==0){
        return;
    }
    else if (coins<=0){
        return;
    }
    new Data[3],szTemp[512]
    Data[0] = get_func_id(callbackFunc);
    Data[1] = id;
    Data[2] = coins;
    format(szTemp,charsmax(szTemp),"insert into clan_donations (user_id,clan_id,amount) values (%d,%d,%d);",user_id,member_in_clan,coins)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_donate_callback",szTemp,Data,1)


}

public _____clan_cts_donate_callback(FailState, Handle:Query, error[], errorCode, data[], dataSize) {
    new id=data[1]
    if (FailState != TQUERY_SUCCESS) {
        client_print_color(id, print_team_red, "%s ^3can't donate ^1!!",CTS_CLAN_PREFIX)
        SQL_FreeHandle(Query);
        return;
    }
    new balance=data[2]

    callfunc_begin_i(data[0]);
    callfunc_push_int(id);
    callfunc_push_int(balance);
    callfunc_end();
    SQL_FreeHandle(Query);
}

/**
 * Lets a player leave their current clan.
 *
 * @param id           The player's ID.
 */
stock clan_cts_left_the_clan(id){
    new user_id = ____cts_db_users[id] , member_in_clan = ____cts_db_member_in_clan_of[id]
    if (user_id==0){
        return;
    }
    else if (member_in_clan==0){
        return;
    }
    new Data[1],szTemp[512]
    Data[0] = id
    format(szTemp,charsmax(szTemp),"delete from clan_members where user_id=%d and clan_id=%d;",user_id,member_in_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
    ____cts_db_member_in_clan_of[id]=0

}

/**
 * Changes the current clan leader by the must person has more donation or by the old person in clan.
 *
 * @param id           The current leader's ID.
 */
stock clan_cts_change_leader(id){
    new leader_of = ____cts_db_is_leader_of[id],user_id=____cts_db_users[id]
    new member_of_clan=____cts_db_member_in_clan_of[id]
    if(leader_of==0 || user_id==0 || member_of_clan==0 ){
        return ;
    }
    else if(leader_of!=member_of_clan){
        return ;
    }
    new Data[1],szTemp[512]
    Data[0] = id
    format(szTemp,charsmax(szTemp),"UPDATE clans SET leader_id = (SELECT user_id FROM ((SELECT user_id FROM clan_donations WHERE clan_id = %d GROUP BY user_id ORDER BY SUM(amount) DESC LIMIT 1) UNION (SELECT user_id FROM clan_members WHERE clan_id = %d ORDER BY joined_at ASC LIMIT 1)) AS new_leader LIMIT 1) WHERE id = %d;",member_of_clan,member_of_clan,member_of_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
    ____cts_db_is_leader_of[id]=0
}

/**
 * Changes the clan leader to a specific member.
 *
 * @param id           The current leader's ID.
 * @param new_id       The new leader's ID.
 */
stock clan_cts_change_leader_by_he(id,new_id){
    new leader_of = ____cts_db_is_leader_of[id],user_id=____cts_db_users[id]
    new member_of_clan=____cts_db_member_in_clan_of[id]
    new new_member_of_clan=____cts_db_member_in_clan_of[new_id],new_user_id=____cts_db_users[new_id]

    if(leader_of==0 || user_id==0 || member_of_clan==0 || new_member_of_clan==0 || new_user_id==0){
        return ;
    }
    else if(leader_of!=member_of_clan){
        return ;
    }
    else if (new_member_of_clan!=member_of_clan){
        return ;
    }
    new Data[1],szTemp[512]
    Data[0] = id
    format(szTemp,charsmax(szTemp),"UPDATE clans SET leader_id = %d WHERE id = %d;",new_user_id,member_of_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
    ____cts_db_is_leader_of[id]=0
    ____cts_db_is_leader_of[new_id]=member_of_clan
}


/**
 * Changes the clan leader by member vote.
 *
 * @param clan_id      The clan ID.
 * @param user_ids[]   Array of voting user IDs.
 * @param max_member   Maximum number of members (default is 10).
 */
stock clan_cts_change_leader_by_vote(clan_id,user_ids[32],max_member=10){

    new count=0,vote_table_of_user_ids[32]
    for(new i=0;i<max_member;i++)
    {
        if (user_ids[i]>0){
            vote_table_of_user_ids[count]=user_ids[i]
            count++;
        }
    }
    if (count<max_member){
        return ;
    }
    else{
        max_member=count
    }
    new counts[32]
    new i, j, voted_id, max_votes = 0, new_leader = -1

    for (i = 0; i < max_member; i++)
    {
        voted_id = vote_table_of_user_ids[i]

        if (voted_id <= 0)
            continue 
        counts[i] = 1 

        for (j = i + 1; j < max_member; j++)
        {
            if (vote_table_of_user_ids[j] == voted_id)
            {
                counts[i]++
                vote_table_of_user_ids[j] = 0 
            }
        }

        if (counts[i] > max_votes)
        {
            max_votes = counts[i]
            new_leader = voted_id
        }
    }

    if (new_leader <= 0)
    {
        return;
    }
    new szTemp[512]

    format(szTemp,charsmax(szTemp),"UPDATE clans SET leader_id = %d WHERE id = %d;",new_leader,clan_id)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp)

    for (new ii=1;ii<33;ii++){
        if (____cts_db_is_leader_of[ii]==clan_id){
            ____cts_db_is_leader_of[ii]=0
            break
        }
    }
    for (new ii=1;ii<33;ii++){
        if (____cts_db_users[ii]==new_leader){
            ____cts_db_is_leader_of[ii]=clan_id
            break
        }
    }
}

/**
 * Sends a join request to a clan.
 *
 * @param id           The player's ID.
 * @param clan_id      The clan ID to join.
 */
stock clan_cts_join_request(id,clan_id){
    new user_id = ____cts_db_users[id] , member_in_clan = ____cts_db_member_in_clan_of[id]
    if (user_id==0){
        return;
    }
    else if (member_in_clan!=0){
        return;
    }
    new Data[1],szTemp[512]
    Data[0] = id
    format(szTemp,charsmax(szTemp),"insert into clan_invitations (user_id,clan_id) VALUES (%d,%d) ON DUPLICATE KEY UPDATE request_date = NOW();",user_id,clan_id)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)

}

/**
 * Checks a leader's coin record after delete the clan.
 *
 * @param id           The player's ID.
 * @param callbackFunc[] Callback function to be called.
 *
 * @note callback will return callbackFunc(id,record_id,balance,clan_name[],date[])
 */
stock clan_cts_check_coins_record(id,callbackFunc[]){
    new user_id = ____cts_db_users[id]
    if (user_id==0){
        return;
    }
    new Data[2],szTemp[512]
    Data[0] = get_func_id(callbackFunc);
    Data[1] = id;
    format(szTemp,charsmax(szTemp),"SELECT * FROM coins_record WHERE user_id=%d;",user_id)
    SQL_ThreadQuery(____cts_g_SqlTuple,"____clan_cts_check_coins_record_callback",szTemp,Data,2)
}

public ____clan_cts_check_coins_record_callback(FailState, Handle:Query, error[], errorCode, data[], dataSize) {
    new id=data[1]
    if (FailState != TQUERY_SUCCESS) {
        client_print_color(id, print_team_red, "%s ^3error ^1!!",CTS_CLAN_PREFIX)
        SQL_FreeHandle(Query);
        return;
    }
    new id_record,balance,clan_name[100],date[32]
    while(SQL_MoreResults(Query))
    {
        id_record = SQL_ReadResult(Query,0)
        balance = SQL_ReadResult(Query,3)
        SQL_ReadResult(Query,2,clan_name,charsmax(clan_name))
        SQL_ReadResult(Query,4,date,charsmax(clan_name))
       
        callfunc_begin_i(data[0]);
        callfunc_push_int(id);
        callfunc_push_int(id_record);
        callfunc_push_int(balance);
        callfunc_push_str(clan_name)
        callfunc_push_str(date)
        callfunc_end();
    
        SQL_NextRow(Query)
    }
    SQL_FreeHandle(Query);
}

/**
 * delete a coin record by record id.
 *
 * @param id           The record id.
 */
stock clan_cts_delete_coins_record(record_id){
    new szTemp[512]

    format(szTemp,charsmax(szTemp),"DELETE FROM coins_record WHERE id = %d;",record_id)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle",szTemp)
}

/**
 * Gets the top 15 clans.
 *
 * @param id           The player's ID (viewer).
 * @param by           Sorting method (e.g., by donation or members).
 * @param page         Page number for pagination.
 * @param ids[]        Output array of clan IDs.
 * @param names[][]    Output array of clan names.
 * @param image[][]    Output array of clan image URLs.
 * @param prefix[][]   Output array of clan prefixes.
 * @param upgrade_level[] Output array of clan levels.
 * @param Members[]    Output array of member counts.
 * @param count        Output: number of clans returned.
 */
stock clan_cts_top_clans_15(id,by=0,page=0,ids[15],names[15][100],image[15][150],prefix[15][10],upgrade_level[15],Members[15],count){
    new offset=page*15;
    new Handle:SqlConnection =SQL_Connect(____cts_g_SqlTuple);
    new Handle:Query;
    if (by==0){
        Query = SQL_PrepareQuery(SqlConnection,"SELECT c.id,c.name,c.clan_logo_motd,c.clan_prefix,c.upgrade_level,COUNT(cm.user_id) as Members FROM clans c, clan_members cm where c.id=cm.clan_id GROUP BY c.id ORDER BY Members DESC LIMIT 15 OFFSET %d;",offset)
    }
    else{
        Query = SQL_PrepareQuery(SqlConnection,"SELECT c.id,c.name,c.clan_logo_motd,c.clan_prefix,c.upgrade_level,COUNT(cm.user_id) as Members FROM clans c, clan_members cm where c.id=cm.clan_id GROUP BY c.id ORDER BY c.upgrade_level DESC LIMIT 15 OFFSET %d;",offset)
    }
    if(!SQL_Execute(Query))
    {
        client_print_color(id, print_team_red, "%s ^3error ^1!!",CTS_CLAN_PREFIX)
        SQL_FreeHandle(Query);
        return;
    }
    count=0;
    while(SQL_MoreResults(Query))
    {
        ids[count] = SQL_ReadResult(Query,0)
        SQL_ReadResult(Query,1,names[count],99)
        if(SQL_IsNull(Query,2)){
            copy(image[count],149,"")
        }
        else{
            SQL_ReadResult(Query,2,image[count],149)
        }
        SQL_ReadResult(Query,3,prefix[count],9)
        upgrade_level[count] = SQL_ReadResult(Query,4)
        Members[count] = SQL_ReadResult(Query,5)
       
        count++;
        SQL_NextRow(Query)
    }

    SQL_FreeHandle(Query);
    SQL_FreeHandle(g_SqlConnection);
    return;
}

/**
 * Gets the clan's bank coin balance.
 *
 * @param id           The player's ID.
 * @param callbackFunc[] Callback function to be called.
 *
 * @note callback will return callbackFunc(id,balance_coins)
 */
stock clan_cts_get_bank(id,callbackFunc[]){
    new user_id = ____cts_db_users[id],member_in_clan = ____cts_db_member_in_clan_of[id]
    if (user_id==0){
        return;
    }
    else if (member_in_clan==0){
        return;
    }
    new Data[2],szTemp[512]
    Data[0] = get_func_id(callbackFunc);
    Data[1] = id;
    format(szTemp,charsmax(szTemp),"SELECT bank_balance FROM clans WHERE id=%d;",member_in_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"clan_cts_bank_callback",szTemp,Data,2)
}

public clan_cts_bank_callback(FailState, Handle:Query, error[], errorCode, data[], dataSize) {
    new id=data[1]
    if (FailState != TQUERY_SUCCESS) {
        client_print_color(id, print_team_red, "%s ^3error ^1!!",CTS_CLAN_PREFIX)
        SQL_FreeHandle(Query);
        return;
    }
    new balance
    if (SQL_NumResults(Query)>0)
    {
        balance = SQL_ReadResult(Query,0)
 
        callfunc_begin_i(data[0]);
        callfunc_push_int(id);
        callfunc_push_int(balance);
        callfunc_end();
    }
    SQL_FreeHandle(Query);
}

/**
 * Loads all stored clan data.
 */
stock clan_cts_clan_loads(){
    new szTemp[512]
    format(szTemp,charsmax(szTemp),"SELECT id,clan_prefix,clan_logo_motd,upgrade_level,DATEDIFF(last_payment+INTERVAL 14 DAY,now() ) AS daysleft,name FROM clans;")
    SQL_ThreadQuery(____cts_g_SqlTuple,"____clan_cts_clan_loads_callback",szTemp)
}

public ____clan_cts_clan_loads_callback(FailState, Handle:Query, error[], errorCode, data[], dataSize) {
    if (FailState != TQUERY_SUCCESS) {
        SQL_FreeHandle(Query);
        return;
    }
    new id_clan, prefix[10], spryteprint[150], level, daysleft, name[100];
    new id_str[10];

    new JSON:jsono;

    while(SQL_MoreResults(Query))
    {
        id_clan = SQL_ReadResult(Query, 0);
        SQL_ReadResult(Query, 1, prefix, charsmax(prefix));

        if (SQL_IsNull(Query, 2))
            copy(spryteprint, charsmax(spryteprint), "");
        else
            SQL_ReadResult(Query, 2, spryteprint, charsmax(spryteprint));

        level = SQL_ReadResult(Query, 3);
        daysleft = SQL_ReadResult(Query, 4);
        SQL_ReadResult(Query, 5, name, charsmax(name));

        jsono = json_init_object()
        json_object_set_number(jsono, "id", id_clan);
        json_object_set_string(jsono, "prefix", prefix);
        json_object_set_string(jsono, "spryteprint", spryteprint);
        json_object_set_number(jsono, "level", level);
        json_object_set_number(jsono, "daysleft", daysleft);
        json_object_set_string(jsono, "name", name);

        num_to_str(id_clan,id_str,charsmax(id_str));
        json_object_set_value(____cts_clans, id_str, jsono);
        json_free(jsono);

        
        SQL_NextRow(Query)
    }
    SQL_FreeHandle(Query);

}

public ____clan_cts_add_clan_in_json(Handle:Query) {

    new id_clan, prefix[10], spryteprint[150], level, daysleft, name[100],leader_id;
    new id_str[10];

    new JSON:jsono;

    if(SQL_NumResults(Query)>0)
    {
        id_clan = SQL_ReadResult(Query, 0);
        SQL_ReadResult(Query, 1, prefix, charsmax(prefix));

        if (SQL_IsNull(Query, 2))
            copy(spryteprint, charsmax(spryteprint), "");
        else
            SQL_ReadResult(Query, 2, spryteprint, charsmax(spryteprint));

        level = SQL_ReadResult(Query, 3);
        daysleft = SQL_ReadResult(Query, 4);
        SQL_ReadResult(Query, 5, name, charsmax(name));
        leader_id = SQL_ReadResult(Query, 6);
        jsono = json_init_object()
        json_object_set_number(jsono, "id", id_clan);
        json_object_set_string(jsono, "prefix", prefix);
        json_object_set_string(jsono, "spryteprint", spryteprint);
        json_object_set_number(jsono, "level", level);
        json_object_set_number(jsono, "daysleft", daysleft);
        json_object_set_string(jsono, "name", name);
        num_to_str(id_clan,id_str,charsmax(id_str))
        json_object_set_value(____cts_clans, id_str, jsono);
        json_free(jsono);
        for(new i=1;i<33;i++){
            if(leader_id==____cts_db_users[i]){
                ____cts_db_is_leader_of[i]=id_clan
                ____cts_db_member_in_clan_of[i]=id_clan
                client_print_color(0, print_team_blue, "%s ^4%s^1 has create ^3clan ^4%s ^1!!",CTS_CLAN_PREFIX,____cts_db_names[i],name)
                break
            }
        }

    }

}

/**
 * Retrieves detailed information about the player's clan.
 *
 * @param id           The player's ID.
 * @param prefix[]     Output: clan prefix.
 * @param spryteprint[] Output: clan logo/image URL.
 * @param level        Output: clan level.
 * @param daysleft     Output: remaining subscription days.
 * @param name[]       Output: clan name.
 */
stock clan_cts_get_clan_info(id,prefix[10], spryteprint[150], level, daysleft, name[100]){

    if (id==0){
        return false;
    }
    new key[10]
    num_to_str(id,key,9)
    new JSON:object= json_object_get_value(____cts_clans,key)
    if (object==Invalid_JSON ){
        return false;
    }
    json_object_get_string(object,"prefix",prefix,charsmax(prefix))
    json_object_get_string(object,"spryteprint",spryteprint,charsmax(spryteprint))
    level=json_object_get_number(object,"level")
    daysleft=json_object_get_number(object,"daysleft")
    json_object_get_string(object,"name",name,charsmax(name))
    json_free(object);
    return true;

}

/**
 * Gets the top 15 clan donators.
 *
 * @param id           The viewer's ID.
 * @param page         Page number.
 * @param user_ids[]   Output: user IDs.
 * @param user_names[][] Output: usernames.
 * @param steam_ids[][] Output: Steam IDs.
 * @param total_donations[] Output: total donations per user.
 * @param count        Output: number of results returned.
 */
stock clan_cts_top_donators_15(id,page=0,user_ids[15],user_names[15][32],steam_ids[15][32],total_donations[15],count){
    new user_id = ____cts_db_users[id],member_in_clan = ____cts_db_member_in_clan_of[id]
    if (user_id==0){
        return;
    }
    else if (member_in_clan==0){
        return;
    }

    new offset=page*15;

    //format(szTemp,charsmax(szTemp),"SELECT u.user_id,user_name,steam_id,clan_id,total_donations FROM clan_donations_temp cdt, users u WHERE cdt.user_id=u.user_id and clan_id=%d ORDER BY total_donations DESC LIMIT 15 OFFSET %d;",member_in_clan,offset)
    new Handle:SqlConnection =SQL_Connect(____cts_g_SqlTuple);
    new Handle:Query = SQL_PrepareQuery(SqlConnection,"SELECT u.user_id,user_name,steam_id,clan_id,total_donations FROM clan_donations_temp cdt, users u WHERE cdt.user_id=u.user_id and clan_id=%d ORDER BY total_donations DESC LIMIT 15 OFFSET %d;",member_in_clan,offset)
    if(!SQL_Execute(Query))
    {
        client_print_color(id, print_team_red, "%s ^3error ^1!!",CTS_CLAN_PREFIX)
        SQL_FreeHandle(Query);
        return;
    }
    count=0;
    while(SQL_MoreResults(Query))
    {
        user_ids[count] = SQL_ReadResult(Query,0)
        SQL_ReadResult(Query,1,user_names[count],31)
        SQL_ReadResult(Query,2,steam_ids[count],31)
        //clan_id = SQL_ReadResult(Query,3)
        total_donations[count] = SQL_ReadResult(Query,4)
        count++;
        SQL_NextRow(Query)
    }
    //SQL_ThreadQuery(____cts_g_SqlTuple,"clan_cts_top_donators_15_callback",szTemp,Data,2)
    SQL_FreeHandle(Query);
    SQL_FreeHandle(g_SqlConnection);
    return;
}

/**
 * Shows up to 15 join requests to a clan.
 *
 * @param id           The clan leader's ID.
 * @param page         Page number.
 * @param user_ids[]   Output: user IDs of requesters.
 * @param user_names[][] Output: usernames of requesters.
 * @param steam_ids[][] Output: Steam IDs.
 * @param dates[][]    Output: date of request.
 * @param count        Output: number of requests returned.
 */
stock clan_cts_show_requests_15(id,page=0,user_ids[15],user_names[15][32],steam_ids[15][32],dates[15][32],count=0){
    new user_id = ____cts_db_users[id],member_in_clan = ____cts_db_member_in_clan_of[id]
    if (user_id==0){
        return;
    }
    else if (member_in_clan==0){
        return;
    }


    new offset=page*15;

    //SQL_ThreadQuery(____cts_g_SqlTuple,"clan_cts_show_requests_15_callback",szTemp,Data,2)
    new Handle:SqlConnection =SQL_Connect(____cts_g_SqlTuple);
    new Handle:Query = SQL_PrepareQuery(SqlConnection,"SELECT ci.user_id,u.user_name,u.steam_id,clan_id,request_date FROM clan_invitations ci , users u where ci.user_id=u.user_id and clan_id=%d ORDER BY request_date DESC LIMIT 15 OFFSET %d;",member_in_clan,offset)
    if(!SQL_Execute(Query))
    {
        client_print_color(id, print_team_red, "%s ^3error ^1!!",CTS_CLAN_PREFIX)
        SQL_FreeHandle(Query);
        return;
    }
    count=0;
    while(SQL_MoreResults(Query))
    {
        user_ids[count] = SQL_ReadResult(Query,0)
        SQL_ReadResult(Query,1,user_names[count],31)
        SQL_ReadResult(Query,2,steam_ids[count],31)
        //clan_id = SQL_ReadResult(Query,3)
        SQL_ReadResult(Query,4,dates[count],31)
        count++;
        SQL_NextRow(Query)
    }
    SQL_FreeHandle(Query);
    SQL_FreeHandle(g_SqlConnection);
    return;
}

/**
 * Returns list of online players not in a clan.
 *
 * @param players[]    Output: player indexes.
 * @param user_ids[]   Output: user IDs.
 * @param num          Output: total number found.
 */
stock clan_cts_get_online_members_no_clan(players[32],user_ids[32],num){
    num=0;
    for (new i=1;i<33;i++){
        if ((____cts_db_users[i]!=0) &&(____cts_db_member_in_clan_of[i]==0) && is_user_connected(i)){
            players[num]=i;
            user_ids[num]=____cts_db_users[i];
            num++;
        }
    }
}

/**
 * Returns list of online players in the player's clan.
 *
 * @param id           The player's ID.
 * @param players[]    Output: player indexes.
 * @param user_ids[]   Output: user IDs.
 * @param num          Output: total number found.
 */
stock clan_cts_get_online_members_owr_clan(id,players[32],user_ids[32],num){
    new clanid=____cts_db_member_in_clan_of[id]
    num=0;
    for (new i=1;i<33;i++){
        if ((____cts_db_users[i]!=0) &&(____cts_db_member_in_clan_of[i]==clanid) && is_user_connected(i)){
            players[num]=i;
            user_ids[num]=____cts_db_users[i]
            num++;
        }
    }
}


/**
 * Gets internal user ID from player ID.
 *
 * @param id           The player's ID.
 * @return             The user ID.
 */
stock clan_cts_get_player_userid(id){
    return ____cts_db_users[id]
}

/**
 * Gets the clan ID that the player belongs to.
 *
 * @param id           The player's ID.
 * @return             The clan ID.
 */
stock clan_cts_get_player_clanid(id){
    return ____cts_db_member_in_clan_of[id]
}

/**
 * Retrieves the player's name.
 *
 * @param id           The player's ID.
 * @param Output[32]      output string save player's name.
 */
stock clan_cts_get_player_name(id,Output[32]){
    copy(Output,31,____cts_db_names[id])
}

/**
 * Retrieves the player's Steam ID.
 *
 * @param id           The player's ID.
 * @param Output[32]   output string save player's Steam ID.
 */
stock clan_cts_get_player_steamid(id,Output[32]){
    copy(Output,31,____cts_db_steams[id])
}


/**
 * Upgrades the clan's level or features.
 *
 * @param leader       The clan leader's ID.
 */
stock clan_cts_upgrade_clan(leader){
    new leader_of = ____cts_db_is_leader_of[leader],user_id=____cts_db_users[leader]
    new member_of_clan=____cts_db_member_in_clan_of[leader]
    if(leader_of==0 || user_id==0 || member_of_clan==0 ){
        return ;
    }
    else if(leader_of!=member_of_clan){
        return ;
    }


    new Data[1],szTemp[512]
    Data[0] = leader
    format(szTemp,charsmax(szTemp),"UPDATE clans SET bank_balance = bank_balance - CASE WHEN upgrade_level = 0 THEN 300 WHEN upgrade_level = 1 THEN 500 ELSE 0 END,upgrade_level = upgrade_level + 1,last_payment = NOW() WHERE id = %d and upgrade_level < 2 AND ((upgrade_level = 0 and bank_balance>=300) or (upgrade_level = 1 and bank_balance>=500))", member_of_clan)
    SQL_ThreadQuery(____cts_g_SqlTuple,"_____clan_cts_IgnoreHandle_Edited",szTemp,Data,1)
}


/**
 * Logs out the player from the clan system.
 *
 * @param id           The player's ID.
 */
stock clan_cts_player_logout(id)
{
    ____cts_db_users[id]=0
    ____cts_db_member_in_clan_of[id]=0
    ____cts_db_is_leader_of[id]=0
    copy(____cts_db_names[id],31,"")
    copy(____cts_db_steams[id],31,"")
}

