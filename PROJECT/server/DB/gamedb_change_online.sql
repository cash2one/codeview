DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_SetVipLv`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_SetVipLv`(pUserName varchar(18),pVipLv tinyint unsigned)
BEGIN
	update t_actors set VipLevel=pVipLv where Name=pUserName;
    END$$

DELIMITER ;


alter table `t_firstrechargeed` add column `level` tinyint UNSIGNED DEFAULT '0' NOT NULL after `UserID`;



DELIMITER $$

DROP PROCEDURE IF EXISTS `P_GDB_Get_IsGetFirstRechargeForward`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_IsGetFirstRechargeForward`(pUserID int unsigned,puidUser bigint unsigned,pLevel tinyint unsigned)
BEGIN
	declare t_userid int unsigned default 0;
	select UserID into t_userid from t_firstrechargeed where UserID=pUserID;
	if t_userid = 0 then
		select 0;
		select puidUser;
		insert into t_firstrechargeed(UserID,level) values(pUserID,pLevel);
	else
		select 2;
		select 0;
	end if;
    END$$

DELIMITER ;
