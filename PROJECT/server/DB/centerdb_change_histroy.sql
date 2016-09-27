
/************************************************2012 -02-18 充值返利 ***************************************************/

DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_Get_TotalRecharge`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_TotalRecharge`(pUserID int unsigned,pBeginTime int unsigned, pEndTime int unsigned,pServerID smallint unsigned)
BEGIN
	declare t_totalrecharge int unsigned default 0;
	declare t_begintime datetime;
	declare t_endtime datetime;
	declare t_userid int unsigned default 0;
	select userid into t_userid from t_rechargelog where userid=pUserID and serverid=pServerID and rechargetime >= t_begintime and rechargetime <= t_endtime limit 0,1;
	if t_userid = 0 then
		select 2;
		select 0;
	else
		set t_begintime = FROM_UNIXTIME(pBeginTime);
		set t_endtime = FROM_UNIXTIME(pEndTime);
		select SUM(amount) into t_totalrecharge from t_rechargelog where userid=pUserID and serverid=pServerID and rechargetime >= t_begintime and rechargetime <= t_endtime;
		select 0;
		select t_totalrecharge;
	end if;
    END$$

DELIMITER ;

/************************************************2012 -02-18 充值返利 ***************************************************/


/************************************************2012 -02-20 充值返利 ***************************************************/
DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_Get_TotalRecharge`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_TotalRecharge`(pUserID int unsigned,pBeginTime int unsigned, pEndTime int unsigned,pServerID smallint unsigned)
BEGIN
	declare t_totalrecharge int unsigned default 0;
	declare t_begintime datetime;
	declare t_endtime datetime;
	declare t_userid int unsigned default 0;
	set t_begintime = FROM_UNIXTIME(pBeginTime);
	set t_endtime = FROM_UNIXTIME(pEndTime);
	select userid into t_userid from t_rechargelog where userid=pUserID and serverid=pServerID and rechargetime >= t_begintime and rechargetime <= t_endtime limit 0,1;
	if t_userid = 0 then
		select 2;
		select 0;
	else
		select SUM(amount) into t_totalrecharge from t_rechargelog where userid=pUserID and serverid=pServerID and rechargetime >= t_begintime and rechargetime <= t_endtime;
		select 0;
		select t_totalrecharge;
	end if;
    END$$

DELIMITER ;
/************************************************2012 -02-20 充值返利 ***************************************************/




/************************************************2012 -02-21 渠道：添加二级渠道***************************************************/

CREATE TABLE `t_channelmap` (                        
                `SecondChannel` tinyint(4) NOT NULL,               
                `FirstChannel` tinyint(4) NOT NULL,                
                PRIMARY KEY (`SecondChannel`)                      
              ) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

insert  into `t_channelmap`(`SecondChannel`,`FirstChannel`) values (0,0),(1,1),(2,2),(3,3),(4,4),(5,5),(6,6),(7,7),(8,8),(9,9),(10,7),(11,11),(12,12),(13,7),(14,7),(15,7),(16,16),(17,7),(18,7),(19,19),(20,20),(21,21),(22,22),(23,23),(24,24),(25,25),(26,26),(27,27),(28,28),(29,29),(30,7),(31,7),(32,7);

alter table `t_user` add column `SecondChannel` tinyint DEFAULT '0' NULL COMMENT '二级渠道号' after `channel`;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_GetUserInfo`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetUserInfo`(pusername varchar(81),pPwd char(66),pIp varchar(33),pChenal tinyint,pSecret varchar(35))
BEGIN
          declare tuserid  int unsigned default 0;
          declare tpassword char(66);
          declare tSealNoID int unsigned default 0;  -- 封号
          declare pRetCode int;
          declare tuserid2  int unsigned default 0;
          declare tSealNoEnd datetime;
          declare tDeblockingTime datetime;
          declare tIdentityStatus tinyint default 0;
          declare tOnlineTime int default 0;
	  declare tAllow int unsigned default 0;
	  declare tFirstChannel tinyint default -1;
	  select FirstChannel into tFirstChannel from t_channelmap where SecondChannel=pChenal;
	  if tFirstChannel = -1 then
		  insert into t_channelmap(SecondChannel,FirstChannel) values(pChenal,pChenal);
		  select FirstChannel into tFirstChannel from t_channelmap where SecondChannel=pChenal;
	  end if;
	  if tFirstChannel = -1 then
		  select -2;
	  else
		  select UserID,Password,SealNoID,IdentityStatus into tuserid,tpassword,tSealNoID,tIdentityStatus from t_user where UserName = pusername and tFirstChannel=channel;
		  select UserID into tAllow from t_allowenter where UserID = tuserid or UserID = 4294967295 limit 0,1;
		
		  if tuserid = 0 and pChenal <> 0 then
		       insert into t_user(UserName,Password,Secret,channel,SecondChannel,registertime,registerip) values(pusername,pPwd,pSecret,tFirstChannel,pChenal,now(),pIp);
		       select UserID,Password,SealNoID,IdentityStatus into tuserid,tpassword,tSealNoID,tIdentityStatus from t_user where UserName = pusername and tFirstChannel=channel;
		  end if;
		  if tAllow = 0 then
		       set pRetCode = -6;
		
		  elseif tuserid = 0 then
		       set pRetCode = -2; 
	       
		  elseif(pPwd <> tpassword ) then         
		       if pChenal <> 0 then
			 set pRetCode = 1;
			 update t_user set Password = pPwd, Secret= pSecret where UserID = tuserid;
		       else
			 set pRetCode = -3; 
		       end if;
		  else
		       set pRetCode = 1;  -- 鎴愬姛 
		       if tSealNoID <> 0 then
			  select userid,SealNoEnd,DeblockingTime into tuserid2,tSealNoEnd, tDeblockingTime from t_realno where id = tSealNoID;
			  if tuserid2 <> tuserid or now() > tSealNoEnd or (tDeblockingTime is not null and now() > tDeblockingTime) then
			      update t_user set SealNoID = 0 where UserID = tuserid;
			  else
			      set pRetCode = -5;  -- 被封号
			  end if;
		       end if;
		       if pRetCode = 1 then                
			  update t_user set lastlogintime = now(), lastloginip = pIp  where UserID = tuserid ;   
		       end if;
	   
		  end if;
		  -- 最近五小时在线时长,
		  if tIdentityStatus <> 2 and pRetCode = 1 then
		     select sum(unix_timestamp(logoutime)-unix_timestamp(logintime)) into tOnlineTime from t_onlinelog where userid = tuserid and logoutime is not null and unix_timestamp(now()) - unix_timestamp(logoutime) < 5*3600;
		  end if;
	  end if;
        
          select pRetCode ,tuserid,tIdentityStatus,tOnlineTime;        
          select serverid from t_useractor where userid = tuserid;
  END$$

DELIMITER ;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_InsertUserInfo`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_InsertUserInfo`(pusername varchar(81),pPwd char(66),pIp varchar(32),pChanel tinyint unsigned)
BEGIN
	declare tuserid  int unsigned default 0;
	declare tpassword char(66);
	declare pRetCode int;
	declare tFirstChannel tinyint default -1;
	select FirstChannel into tFirstChannel from t_channelmap where SecondChannel=pChanel;
	if tFirstChannel = -1 then
		set pRetCode = -1;
	else
		select UserID into tuserid from t_user where UserName = pusername and channel =tFirstChannel;
		
		if tuserid <> 0 then
		      set pRetCode = -4;  -- 鐢ㄦ埛宸插瓨鍦?         
		else
		   -- 鎻掑叆鏁版嵁
		   insert into t_user(UserName,Password,channel,SecondChannel,registertime,registerip,lastlogintime,lastloginip) 
			values(pusername,pPwd,tFirstChannel,pChanel,now(),pIp,now(),pIp);
		   set tuserid = LAST_INSERT_ID();
		   if tuserid = 0 then
		      set pRetCode = -1;
		   else
		      set pRetCode = 1;  -- 鎴愬姛       
		   end if;       
		end if;
	end if;
	select pRetCode,tuserid,0,0;
	select 0;
    END$$

DELIMITER ;





update t_user set SecondChannel=channel ;

update t_user set channel=(select FirstChannel from t_channelmap where t_channelmap.SecondChannel=t_user.SecondChannel);


/************************************************2012 -02-21 渠道：添加二级渠道***************************************************/


/************************************************2012 -02-28***************************************************/

alter table `t_resource` change `xianstone_num` `xianstone_num` int(8) default '0' NULL  comment '仙石数量', change `lingstone_num` `lingstone_num` int(8) default '0' NULL  comment '灵石数量', change `liquan_num` `liquan_num` int(8) default '0' NULL  comment '礼券数量';

/************************************************2012 -02-28***************************************************/
