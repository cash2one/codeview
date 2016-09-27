alter table `t_vidmap` change `time` `time` datetime default '0000-00-00 00:00:00' NOT NULL;


DELIMITER $$

DROP PROCEDURE IF EXISTS `P_CDB_Insert_PointInfo`$$

CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Insert_PointInfo`(pVid int unsigned,pUserID int unsigned,pOperator tinyint unsigned)
BEGIN
	if pOperator = 1 then
		insert into t_vidmap(user_id,time) values(pUserID,now());
		set pVid = last_insert_id();
	else
		update t_vidmap set user_id=pUserID where v_id=pVid and user_id=0;
	end if;
	insert into t_gamepoint values(pVid,pUserID,pOperator,now());
	select 0;
	select pVid;
    END$$

DELIMITER ;
