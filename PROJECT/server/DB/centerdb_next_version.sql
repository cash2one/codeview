/****************************游戏节点  2013.3.12*********************************************************************/;
CREATE TABLE `t_gamepoint` (                                                                                                        
               `v_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '虚拟ID',                                                               
               `userid` int(10) unsigned NOT NULL COMMENT '玩家ID',                                                                            
               `operator` tinyint(3) unsigned NOT NULL COMMENT '操作：1(检查版本号),2(进入选服页面),3(进入创建角色页面)',  
               `time` datetime NOT NULL COMMENT '操作时间',                                                                                  
               PRIMARY KEY (`v_id`)                                                                                                              
             ) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;


CREATE TABLE `t_vidmap` (                                                       
            `v_id` int(10) unsigned zerofill NOT NULL AUTO_INCREMENT COMMENT '虚拟ID',  
            `user_id` int(10) unsigned NOT NULL COMMENT '玩家ID',                       
            PRIMARY KEY (`v_id`)                                                          
          ) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=gbk COLLATE=gbk_bin;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_Insert_PointInfo`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Insert_PointInfo`(pVid int unsigned,pUserID int unsigned,pOperator tinyint unsigned)
BEGIN
	if pOperator = 1 then
		insert into t_vidmap(user_id) values(pUserID);
		set pVid = last_insert_id();
	end if;
	update t_vidmap set user_id=pUserID where v_id=pVid and user_id=0;
	insert into t_gamepoint values(pVid,pUserID,pOperator,now());
	select 0;
	select pVid;
    END$$

DELIMITER ;


/****************************游戏节点  2013.3.12*********************************************************************/;



alter table `t_gamepoint` change `v_id` `v_id` int(10) UNSIGNED NOT NULL comment '虚拟ID', drop primary key;



alter table `t_onlinelog` add column `level` tinyint DEFAULT '0' NULL COMMENT '下线时的等级' after `logoutgodstone`;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_UserExitGame`$$

CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_UserExitGame`(pServerID smallint unsigned,pUserID int unsigned,pGolStone int,pLevel tinyint unsigned)
BEGIN
          declare tonlinerecordid int unsigned default 0;
           declare tonlinetime int default 0; -- 在线时间
           select onlinerecordid into tonlinerecordid from t_user where userid=pUserID and onlineserverid = pServerID;
           if tonlinerecordid <> 0 then
              select UNIX_TIMESTAMP(now()) - UNIX_TIMESTAMP(logintime) into tonlinetime from t_onlinelog where recordid = tonlinerecordid for update;
              update t_onlinelog set logoutime = now(),logoutgodstone = pGolStone,level=pLevel where recordid = tonlinerecordid;
        
           end if;
           update t_user set onlineserverid = 0,onlinerecordid=0,onlinetime = onlinetime+tonlinetime where userid=pUserID;
    END$$

DELIMITER ;
