
/************************************************2012 -02-18 充值返利 ***************************************************/

CREATE TABLE `t_rechargeforwardcnfg` (                              
                         `BeginTime` datetime NOT NULL,                                    
                         `EndTime` datetime NOT NULL,                                      
                         `szMailTheme` varchar(50) COLLATE gbk_bin NOT NULL DEFAULT '',    
                         `szMailContent` varchar(300) COLLATE gbk_bin NOT NULL DEFAULT ''  
                       ) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

alter table `t_rechargeforward` drop column `BeginTime`, drop column `EndTime`, drop column `szMailTheme`, drop column `szMailContent`;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_RechargeForwardCnfg`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_RechargeForwardCnfg`()
BEGIN
	declare t_begintime int unsigned default 0;
	select UNIX_TIMESTAMP(BeginTime) into t_begintime from t_rechargeforwardcnfg limit 0,1;
	if t_begintime = 0 then
		select 2;
		select 0,0,'','';
	else
		select 0;
		select UNIX_TIMESTAMP(BeginTime),UNIX_TIMESTAMP(EndTime),szMailTheme,szMailContent from t_rechargeforwardcnfg;
	end if;
    END$$

DELIMITER ;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_RechargeForward`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_RechargeForward`()
BEGIN
	declare t_type int default 0;
	select ForwardType into t_type from t_rechargeforward limit 0,1;
	if t_type = 0 then
		select 2;
		select 0,0,0,0,0,0,0,'';
	else
		select 0;
		select MoneyNum,CalType,ForwardType,GodStone,Ticket,Stone,PolyNimbus,ForwardGoods from t_rechargeforward;
	end if;
    END$$

DELIMITER ;

/************************************************2012 -02-18 充值返利 ***************************************************/

/************************************************2012 -02-18 玄天BOSS配置写进配置表 ***************************************************/

CREATE TABLE `t_monstercnfg` (                                                   
                 `MonsterID` smallint(5) unsigned NOT NULL COMMENT '怪物ID',                  
                 `Name` char(18) COLLATE gbk_bin NOT NULL COMMENT '名称',                     
                 `Level` tinyint(3) unsigned NOT NULL COMMENT '怪物等级级别',             
                 `Spirit` int(10) unsigned NOT NULL COMMENT '灵力',                           
                 `Shield` int(10) unsigned NOT NULL COMMENT '护盾',                           
                 `Blood` int(10) unsigned NOT NULL COMMENT '气血',                            
                 `Avoid` int(10) unsigned NOT NULL COMMENT '身法',                            
                 `MagicValue` int(10) unsigned NOT NULL COMMENT '法术值',                    
                 `Swordkee` int(10) unsigned NOT NULL COMMENT '剑气',                         
                 `Facade` smallint(5) unsigned NOT NULL COMMENT '外观',                       
                 `MagicNum` tinyint(3) unsigned NOT NULL COMMENT '法术技能数量',          
                 `MonsterNameLangID` int(10) unsigned NOT NULL COMMENT '怪物名称语言ID',  
                 `Crit` int(11) NOT NULL COMMENT '爆击值',                                   
                 `Tenacity` int(11) NOT NULL COMMENT '坚韧值',                               
                 `Hit` int(11) NOT NULL COMMENT '命中值',                                    
                 `Dodge` int(11) NOT NULL COMMENT '回避值',                                  
                 `MagicCD` int(11) NOT NULL COMMENT '法术回复值',                          
                 PRIMARY KEY (`MonsterID`)                                                      
               ) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_MonsterCnfg`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_MonsterCnfg`()
BEGIN
	declare t_monsterID smallint unsigned default 0;
	select MonsterID into t_monsterID from t_monstercnfg limit 0,1;
	if t_monsterID = 0 then
		select 2;
		select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select MonsterID,Name,Level,Spirit,Shield,Blood,Avoid,MagicValue,Swordkee,Facade,MagicNum,MonsterNameLangID,Crit,Tenacity,Hit,Dodge,MagicCD from t_monstercnfg;
	end if;
    END$$

DELIMITER ;

/************************************************2012 -02-18 玄天BOSS配置写进配置表 ***************************************************/



/************************************************2012 -02-21 充值返利 ***************************************************/

alter table `t_rechargeforward` change `id` `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

/************************************************2012 -02-21 充值返利 ***************************************************/



/************************************************2012 -02-25 活动 ***************************************************/
DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_ActivityCnfgReq`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ActivityCnfgReq`(pActivityID smallint unsigned)
BEGIN
	declare t_activityID smallint unsigned default 0;
	select activityid into t_activityID from t_activitycnfg where activityid=pActivityID;
	if t_activityID = 0 then
		select 2;
		select 0,'',0,'',0,0,0,0,0,'',0,'',0,0,'','','',0;
	else
		select 0;
		select activityid,name,type,rules,UNIX_TIMESTAMP(begintime),UNIX_TIMESTAMP(endtime),resid,bfinish,eventid,param,attainnum,awarddesc,ticket,godstone,vectgoods,mailsubject,mailcontent,ShowOrder from t_activitycnfg where activityid=pActivityID;
	end if;
    END$$

DELIMITER ;
/************************************************2012 -02-25 活动 ***************************************************/


/************************************************2012 -02-26 任务 ***************************************************/
DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_UpdateTask`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_UpdateTask`(pUserUID bigint unsigned, pTaskID smallint unsigned,pFinishTime int unsigned,pCurCount bigint unsigned)
BEGIN
	declare t_TaskID smallint unsigned default 0;
	select TaskID into t_TaskID from t_task where Uid_User=pUserUID and TaskID=pTaskID;
	if t_TaskID = 0 then
		insert into t_task(Uid_User,TaskID,FinishTime,CurCount) values(pUserUID,pTaskID,pFinishTime,pCurCount);
	end if;
    END$$

DELIMITER ;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_DeleteTask`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_DeleteTask`(puidUser bigint unsigned,pTaskID smallint unsigned)
BEGIN
	delete from t_task where Uid_User=puidUser and TaskID=pTaskID;
    END$$

DELIMITER ;

/************************************************2012 -02-26 任务 ***************************************************/

/************************************************2012 -02-26 帮派默认值 ***************************************************/

alter table `t_syndicate` change `SynWarTotalScore` `SynWarTotalScore` int(10) UNSIGNED default '0' NOT NULL;

/************************************************2012 -02-26 帮派默认值 ***************************************************/

/************************************************2012 -02-26 夺宝等级默认值 ***************************************************/

alter table `t_challengerank` change `DuoBaoLevel` `DuoBaoLevel` int(11) default '0' NOT NULL;

/************************************************2012 -02-26 夺宝等级默认值 ***************************************************/



/*********************************2013-02-28*************************************************************************************/

DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Insert_SysMailByUserID`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_SysMailByUserID`(pUserID int unsigned, puidSendUser bigint unsigned,pNameSendUser varchar(18), pMailType tinyint unsigned,
								pStone int unsigned,pMoney int unsigned,pTicket int unsigned,pPolyNimbus int unsigned,pThemeText varchar(50), pContentText varchar(300),
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


alter table `t_rechargeforwardcnfg` add column `isgive` tinyint(2) DEFAULT '0' NULL after `szMailContent`;

/*********************************2013-02-28*************************************************************************************/
