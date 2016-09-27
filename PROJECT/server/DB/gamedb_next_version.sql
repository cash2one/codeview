
/******************************************************************2013-03-12 begin****************************************************************/


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_MyChallengeRecord`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_MyChallengeRecord`(pUidUser bigint unsigned, pNum int unsigned)
BEGIN
	declare t_ranknum int unsigned default 0;
	select count(Rank) into t_ranknum from t_mychallengerecord where uidUser = pUidUser;
	if t_ranknum = 0 then
		select 2;
		select 0,0,0,'',0,0,0,0,0;
	else
		select 0;
		if pNum > t_ranknum  then
			set pNum = t_ranknum;
		end if;
		
		select uidUser,uidEnemy,Rank,EnemyName,EnemyLevel,bWin,bActive,VipLevel, EnemyFacade from t_mychallengerecord where uidUser = pUidUser order by Rank asc limit 0,pNum;
	end if;
    END$$

DELIMITER ;




/**********************************************************************************************************************************/



create table `t_talismanrecordbuf`( `Rank` int(10) UNSIGNED NOT NULL , `RecordBuf` blob NOT NULL , `BufLen` int(10) NOT NULL )  ;
create table `t_talismanrecord`( `Rank` int(10) UNSIGNED NOT NULL AUTO_INCREMENT , `uidUser` bigint(20) UNSIGNED NOT NULL , `uidEnemy` bigint(20) UNSIGNED NOT NULL , `EnemyName` char(18) CHARSET gbk COLLATE gbk_bin NOT NULL , `VipLevel` tinyint(3) UNSIGNED NOT NULL , `GhostSoul` int(10) UNSIGNED NOT NULL , PRIMARY KEY (`Rank`))  ;

DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Update_Occupation_Info`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_Occupation_Info`(pUidUser bigint unsigned, pUidEnemy bigint unsigned,pBufLen int unsigned,pRecordBuf blob(40960))
BEGIN
	declare t_rank int unsigned default 0;
	declare t_maxrank int unsigned default 0;
	declare t_VipLevel tinyint unsigned default 0;
	declare t_NameEnemy char(18) default "";
	select  Rank into t_rank from t_talismanrecord where uidUser = pUidUser ;
	select Name,VipLevel into t_NameEnemy,t_VipLevel from t_actors where uid = pUidEnemy;
	if t_rank = 0 then
		select max(Rank) into t_maxrank from t_talismanrecord;
		if t_maxrank is null then
		   set t_maxrank = 0;
		end if;
		set t_maxrank = t_maxrank + 1;

		insert into t_talismanrecord(Rank,uidUser,uidEnemy,EnemyName,VipLevel) values(t_maxrank,pUidUser,pUidEnemy, t_NameEnemy,t_VipLevel);
		insert into t_talismanrecordbuf(Rank,RecordBuf,BufLen) values (t_maxrank,pRecordBuf,pBufLen);
		select 0;
		select t_maxrank;
	else
		update t_talismanrecord set uidEnemy = pUidEnemy,VipLevel = t_VipLevel,EnemyName = t_NameEnemy where uidUser = pUidUser;
		update t_talismanrecordbuf set RecordBuf = pRecordBuf,BufLen = pBufLen where Rank = t_rank;

		select 0;
		select t_rank;		
	end if;

		
    END$$

DELIMITER ;

/**********************************************************************************************************************************/

DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Update_Occupation_GhostSoul`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_Occupation_GhostSoul`(pUidUser bigint unsigned,pGhostSoul int unsigned)
BEGIN
	
		update t_talismanrecord set GhostSoul = pGhostSoul where uidUser = pUidUser;
		
    END$$

DELIMITER ;

/**********************************************************************************************************************************/

DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_OccupationInfo`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_OccupationInfo`(pUidUser bigint unsigned)
BEGIN
	declare t_rank int unsigned default 0;
	select Rank into t_rank from t_talismanrecord where uidUser = pUidUser ;
	if t_rank = 0 then
		select 2;
		select 0,'',0,0;
	else
		select 0;		
		select Rank,EnemyName,VipLevel,GhostSoul from t_talismanrecord where uidUser = pUidUser;
	end if;
    END$$

DELIMITER ;



/******************************************************************2013-03-12 end****************************************************************/



/****************************练功与习武  2013.3.12*********************************************************************/;
alter table `t_actors` add column `LastOnlineTime` int UNSIGNED DEFAULT '0' NOT NULL after `SoilDamage`;



DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_GetActorBasicInfo`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetActorBasicInfo`(pUserID int unsigned)
BEGIN
          declare t_userid int unsigned default 0;
 select UserID into t_userid from t_allowenter where UserID = pUserID or UserID=0xffffffff limit 0,1;
      if t_userid = 0 then
          select 6;
          select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
       else
	  set t_userid = 0;
          select UserID into t_userid from t_actors where UserID = pUserID;
          if t_userid = 0 then
                    select 2;
		    select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
          else
                    select 0;
		    select * from t_actors where UserID = pUserID;
          end if;
       end if;
    END$$

DELIMITER ;



DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_UpdateActorInfo2`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateActorInfo2`(pUserID int unsigned,pName varchar(18),pLevel tinyint unsigned,pSpirit int,pShield int,pAvoid int,pActorExp int,pActorLayer int,pActorNimbus int,pActorAptitude int,pActorSex tinyint unsigned,puid bigint unsigned,pActorMoney int,pActorTicket int,pActorStone int,
							pActorFacade smallint unsigned,pActorBloodUp int,pDir tinyint unsigned,pptX smallint unsigned,pptY smallint unsigned,puidMaster bigint unsigned,pCityID smallint unsigned, pHonor int, pCredit int,pActorNimbusSpeed int,pGodSwordNimbus int,pUseFlag Binary(2),pVipLevel tinyint unsigned,pRecharge int,
							pActorPolyNimbus int,pGoldDamageLv tinyint unsigned,pWoodDamageLv tinyint unsigned,pWaterDamageLv tinyint unsigned,pFireDamageLv tinyint unsigned,
							pSoilDamageLv tinyint unsigned,pSynCombatLevel int,pCritLv tinyint unsigned,pTenacityLv tinyint unsigned,pHitLv tinyint unsigned,pDodgeLv tinyint unsigned,
							pMagicCDLv tinyint unsigned,pCrit int,pTenacity int,pHit int,pDodge int,pMagicCD int,
							pGhostSoul int,pDuoBaoLevel int,pCombatAbility int unsigned,pGoldDamage int,pWoodDamage int,pWaterDamage int,pFireDamage int,pSoilDamage int)
BEGIN
	declare t_uid bigint unsigned default 0;
	select uid into t_uid from t_actors where uid=puid;
 	if t_uid = 0 then
		insert into t_actors values(pUserID,pName,pLevel,pSpirit,pShield,pAvoid,pActorExp,pActorLayer,pActorNimbus,pActorAptitude,pActorSex,puid,pActorMoney,pActorTicket,pActorStone,pActorFacade,pActorBloodUp,pDir,pptX,pptY,puidMaster,pCityID,pHonor,pCredit,pActorNimbusSpeed,pGodSwordNimbus,pUseFlag,pVipLevel,pRecharge,
					pActorPolyNimbus,pGoldDamageLv,pWoodDamageLv,pWaterDamageLv,pFireDamageLv,pSoilDamageLv,pSynCombatLevel,pCritLv,pTenacityLv,pHitLv,pDodgeLv,pMagicCDLv,pCrit,pTenacity,pHit,pDodge,pMagicCD,pGhostSoul,pDuoBaoLevel,pCombatAbility,pGoldDamage,pWoodDamage,pWaterDamage,pFireDamage,pSoilDamage,0);
	else
		update t_actors set Name=pName,Level=pLevel,Spirit=pSpirit,Shield=pShield,Avoid=pAvoid,ActorExp=pActorExp,ActorLayer=pActorLayer,ActorNimbus=pActorNimbus,ActorAptitude=pActorAptitude,ActorSex=pActorSex,ActorMoney=pActorMoney,ActorTicket=pActorTicket,ActorStone=pActorStone,ActorFacade=pActorFacade,
						ActorBloodUp=pActorBloodUp,nDir=pDir,ptX=pptX,ptY=pptY,uidMaster=puidMaster,CityID=pCityID,Honor=pHonor,Credit=pCredit,ActorNimbusSpeed=pActorNimbusSpeed,GodSwordNimbus=pGodSwordNimbus,UserFlag=pUseFlag,VipLevel=pVipLevel,Recharge=pRecharge,
						ActorPolyNimbus=pActorPolyNimbus,GoldDamageLv=pGoldDamageLv,WoodDamageLv=pWoodDamageLv,WaterDamageLv=pWaterDamageLv,FireDamageLv=pFireDamageLv,SoilDamageLv=pSoilDamageLv,SynCombatLevel=pSynCombatLevel,
						CritLv=pCritLv,TenacityLv=pTenacityLv,HitLv=pHitLv,DodgeLv=pDodgeLv,MagicCDLv=pMagicCDLv,Crit=pCrit,Tenacity=pTenacity,Hit=pHit,Dodge=pDodge,MagicCD=pMagicCD,GhostSoul=pGhostSoul,DuoBaoLevel=pDuoBaoLevel, CombatAbility = pCombatAbility, GoldDamage = pGoldDamage, WoodDamage = pWoodDamage, WaterDamage = pWaterDamage, FireDamage = pFireDamage, SoilDamage = pSoilDamage where uid=puid;
		update t_challengerank set level = pLevel,layer = pActorLayer,viplevel=pVipLevel,DuoBaoLevel = pDuoBaoLevel where t_challengerank.uidUser=puid;
		update t_mychallengerecord set EnemyLevel = pLevel,VipLevel = pVipLevel where t_mychallengerecord.uidEnemy = puid;
	 end if;
    END$$

DELIMITER ;



DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_GetActorBasicDataByUIDInfo`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_GetActorBasicDataByUIDInfo`(pUserUid bigint unsigned)
BEGIN
          declare t_uid bigint unsigned default 0;
          select uid into t_uid from t_actors where uid = pUserUid;
          if t_uid = 0 then
                    select 2;  -- 鐟欐帟澹婃稉宥呯摠閸? 
		    select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
          else
                    select 0;   -- 閹存劕濮?
		    select * from t_actors where uid = pUserUid;
          end if;
    END$$

DELIMITER ;



DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_GetEmployeeInfo`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetEmployeeInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_num tinyint unsigned default 0;
	select count(*) into t_num from t_actors where uidMaster = pUid_User;
	if t_num = 0 then
		select 0;
		select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
	else
		select t_num;
		select * from t_actors where uidMaster = pUid_User;
	end if;
    END$$

DELIMITER ;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Update_LastOnlineTime`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_LastOnlineTime`(puidUser bigint unsigned,pLastOnlineTime int unsigned)
BEGIN
	update t_actors set LastOnlineTime=pLastOnlineTime where uid=puidUser;
    END$$

DELIMITER ;



alter table `t_traininghall` add column `TrainingTimeLong` int UNSIGNED DEFAULT '0' NOT NULL after `LastVipFinishTrainTime`;

alter table `t_traininghall` add column `BeginXiWuTime` int UNSIGNED DEFAULT '0' NOT NULL after `TrainingTimeLong`, add column `uidActorXiWu` bigint UNSIGNED DEFAULT '0' NOT NULL after `BeginXiWuTime`;

alter table `t_traininghall` drop column `GetExp`;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_UpdateTrainingHallInfo`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateTrainingHallInfo`(pUid_User bigint unsigned,pRemainNum smallint unsigned,pBeginTime int unsigned,pLastFinishTime int unsigned, pStatus tinyint unsigned,pVipFinishTrainNum tinyint unsigned,
					pLastVipFinishTrainTime int unsigned,pTrainingTimeLong int unsigned,pBeginXiWuTime int unsigned,puidActorXiWu bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_traininghall where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_traininghall values(pUid_User,pRemainNum,pBeginTime,pLastFinishTime,pStatus,pVipFinishTrainNum,pLastVipFinishTrainTime,pTrainingTimeLong,pBeginXiWuTime,puidActorXiWu);
	else
		update t_traininghall set RemainNum=pRemainNum,BeginTime=pBeginTime,LastFinishTime=pLastFinishTime,TrainStatus=pStatus,VipFinishTrainNum=pVipFinishTrainNum,LastVipFinishTrainTime=pLastVipFinishTrainTime,TrainingTimeLong=pTrainingTimeLong,BeginXiWuTime=pBeginXiWuTime,uidActorXiWu=puidActorXiWu where Uid_User = pUid_User;
	end if;
    END$$

DELIMITER ;

DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_GetTrainingHallInfo`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetTrainingHallInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_traininghall where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select pUid_User,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_traininghall where Uid_User=pUid_User;
	end if;
    END$$

DELIMITER ;

/****************************练功与习武  2013.3.12*********************************************************************/;



CREATE TABLE `t_xtbossdielog` (                                        
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,                       
                  `BossName` char(18) COLLATE gbk_bin NOT NULL,                        
                  `DieDate` datetime NOT NULL,                                         
                  PRIMARY KEY (`Id`)                                                   
                ) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=gbk COLLATE=gbk_bin;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Save_XTBossDie`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Save_XTBossDie`(pUIDBoss bigint unsigned, pUIDUser bigint unsigned, pUserName char(18), pBossName char(18),pTotalDamage int,pCurDamage int, pAttackNum int, pUserLevel tinyint unsigned)
BEGIN
	declare t_maxid int unsigned default 0;
	insert into t_xtbossdie(UIDBoss,UIDUser,UserName,BossName,TotalDamage,CurDamage,attacknum,UserLevel) values(pUIDBoss,pUIDUser,pUserName,pBossName,pTotalDamage,pCurDamage,pAttackNum,pUserLevel);
	insert into t_xtbossdielog(BossName,DieDate) value(pBossName,now());
	select max(Id) into t_maxid from t_xtbossdielog;
	if t_maxid = 0 or t_maxid = NULL then
		set t_maxid = 1;
	end if;
	
	if t_maxid > 20 then
		delete from t_xtbossdielog where Id + 20 <= t_maxid;
	end if;
    END$$

DELIMITER ;


alter table `t_mail` change `ContentText` `ContentText` varchar(600) character set gbk collate gbk_bin NOT NULL;



DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Insert_MailData_Req`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_MailData_Req`(pUid_User bigint unsigned, pUid_SendUser bigint unsigned, pSendUserName varchar(18),  pMail_Type tinyint unsigned,pStone int unsigned, pMoney int unsigned,
                   pTicket int unsigned, pPolyNimbus int unsigned,pSynID smallint unsigned, pThemeText varchar(50), 
                    pContentText varchar(600), pTime bigint unsigned , pGoodsData varbinary(1024))
BEGIN
	declare t_MailID int unsigned default 0;
	declare t_NumRecord int unsigned default 0;
	declare t_Time bigint	unsigned default 0;
	
        select  MailID into t_MailID from t_mail where uid_User = pUid_User order by MailID desc limit 300,1;
        if t_MailID <> 0 then
           delete from t_mail where MailID = t_MailID;
         end if;
       	
	insert into t_mail(uid_User, uid_SendUser, Name_SendUser, Mail_Type,stone, money, ticket,PolyNimbus, SynID, ThemeText, ContentText, Time, GoodsData) 
          values(pUid_User, pUid_SendUser, pSendUserName, pMail_Type,pStone, pMoney, pTicket,pPolyNimbus, pSynID, pThemeText, pContentText, pTime, pGoodsData);
	
	set t_MailID = LAST_INSERT_ID();
	if t_MailID = 0 then
		select 3;
		select t_MailID,pUid_User;
	else
		select 0;
		select t_MailID,pUid_User;
	end if;
    END$$

DELIMITER ;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Insert_SysMailByUserID`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_SysMailByUserID`(pUserID int unsigned, puidSendUser bigint unsigned,pNameSendUser varchar(18), pMailType tinyint unsigned,
								pStone int unsigned,pMoney int unsigned,pTicket int unsigned,pPolyNimbus int unsigned,pThemeText varchar(50), pContentText varchar(600),
								pTime bigint unsigned,pGoodsData varbinary(1024))
BEGIN
	declare t_uid bigint unsigned default 0;
	select uid into t_uid from t_actors where UserID = pUserID;
	
	if t_uid <> 0 then
		insert into t_mail(uid_User,uid_SendUser,Name_SendUser,Mail_Type,stone,money,ticket,PolyNimbus,SynID,ThemeText,ContentText,Time,GoodsData)
		values(t_uid, puidSendUser,pNameSendUser,pMailType,pStone,pMoney,pTicket,pPolyNimbus,0,pThemeText,pContentText,pTime,pGoodsData);
	end if;
    END$$

DELIMITER ;

alter table `t_sysmail` change `Content` `Content` varchar(600) character set gbk collate gbk_bin NULL  comment '正文';
alter table `t_rechargeforwardcnfg` change `szMailContent` `szMailContent` varchar(600) character set gbk collate gbk_bin default '' NOT NULL;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_WriteAllUserSysMail`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_WriteAllUserSysMail`(pUidSendUser bigint unsigned,pNameSendUser varchar(18), pMailType tinyint unsigned,pStone int unsigned,
								pMoney int unsigned,pTicket int unsigned,pPolyNimbus int unsigned,pThemeText varchar(50), pContentText varchar(600), 
								pTime bigint unsigned,pGoodsData varbinary(1024))
BEGIN
	insert into t_mail(uid_User,uid_SendUser,Name_SendUser,Mail_Type,stone,money,ticket,PolyNimbus,SynID,ThemeText,ContentText,Time,GoodsData)
		select uid,pUidSendUser,pNameSendUser,pMailType,pStone,pMoney,pTicket,pPolyNimbus,0,pThemeText,pContentText,pTime,pGoodsData from t_actors where t_actors.uidMaster = 0;
    END$$

DELIMITER ;


/****************************首次充值  2013.3.22*********************************************************************/;
CREATE TABLE `t_firstrechargecnfg` (                                                    
                       `RateTicket` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '礼卷比例',         
                       `RateGodStone` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '仙石比例',       
                       `MinRecharge` int(11) NOT NULL DEFAULT '0' COMMENT '有奖励的最小充值金额',  
                       `MailTheme` varchar(50) COLLATE gbk_bin NOT NULL DEFAULT '' COMMENT '邮件主题',   
                       `MailContent` varchar(600) COLLATE gbk_bin DEFAULT '' COMMENT '邮件内容'          
                     ) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;


CREATE TABLE `t_firstrechargeed` (                   
                     `UserID` int(10) unsigned NOT NULL,                
                     PRIMARY KEY (`UserID`)                             
                   ) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_FirstRechargeCnfg`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_FirstRechargeCnfg`()
BEGIN
	declare t_ticket tinyint default -1;
	select RateTicket into t_ticket from t_firstrechargecnfg limit 0,1;
	if t_ticket = -1 or t_ticket=NULL then
		select 2;
		select 0,0,0,'','';
	else
		select 0;
		select RateTicket,RateGodStone,MinRecharge,MailTheme,MailContent from t_firstrechargecnfg limit 0,1;
	end if;
    END$$

DELIMITER ;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_IsGetFirstRechargeForward`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_IsGetFirstRechargeForward`(pUserID int unsigned,puidUser bigint unsigned)
BEGIN
	declare t_userid int unsigned default 0;
	select UserID into t_userid from t_firstrechargeed where UserID=pUserID;
	if t_userid = 0 then
		select 0;
		select puidUser;
		insert into t_firstrechargeed(UserID) values(pUserID);
	else
		select 2;
		select 0;
	end if;
    END$$

DELIMITER ;

/****************************首次充值  2013.3.22*********************************************************************/;


/****************************老玩家充过值的写到首次充值表中 2013.3.27*********************************************************************/;
insert into t_firstrechargeed(UserID) select UserID from t_actors where Recharge>0;
/****************************老玩家充过值的写到首次充值表中 2013.3.27*********************************************************************/;


