锘�*
SQLyog Enterprise - MySQL GUI v7.14 
MySQL - 5.5.19 : Database - gamedb
*********************************************************************
*/
/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

CREATE DATABASE /*!32312 IF NOT EXISTS*/`gamedb` /*!40100 DEFAULT CHARACTER SET gbk COLLATE gbk_bin */;

USE `gamedb`;

/*Table structure for table `bigint_test` */

DROP TABLE IF EXISTS `bigint_test`;

CREATE TABLE `bigint_test` (
  `bigint_col` bigint(20) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_activity` */

DROP TABLE IF EXISTS `t_activity`;

CREATE TABLE `t_activity` (
  `uidActor` bigint(20) unsigned NOT NULL DEFAULT '0',
  `SignInNumOfMonth` tinyint(3) unsigned DEFAULT '0',
  `LastSignInTime` int(10) unsigned DEFAULT '0',
  `AwardHistory` varbinary(4) DEFAULT '0',
  `OnLineAwardID` smallint(5) unsigned DEFAULT '0',
  `LastOnLineAwardRestTime` int(10) unsigned DEFAULT '0',
  `bCanTakeOnlineAward` tinyint(1) DEFAULT '0',
  `OffLineNum` tinyint(1) DEFAULT '0',
  `NewPlayerGuideContext` varchar(1024) COLLATE gbk_bin DEFAULT '0',
  PRIMARY KEY (`uidActor`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_activitycnfg` */

DROP TABLE IF EXISTS `t_activitycnfg`;

CREATE TABLE `t_activitycnfg` (
  `activityid` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` char(18) COLLATE gbk_bin NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rules` varchar(300) COLLATE gbk_bin NOT NULL,
  `begintime` datetime NOT NULL,
  `endtime` datetime NOT NULL,
  `resid` smallint(5) unsigned NOT NULL,
  `bfinish` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `eventid` smallint(5) unsigned NOT NULL,
  `param` char(100) COLLATE gbk_bin NOT NULL,
  `attainnum` smallint(6) NOT NULL,
  `awarddesc` varchar(300) COLLATE gbk_bin NOT NULL,
  `ticket` smallint(6) NOT NULL,
  `godstone` smallint(6) NOT NULL,
  `vectgoods` varchar(300) COLLATE gbk_bin NOT NULL,
  `mailsubject` varchar(50) COLLATE gbk_bin NOT NULL,
  `mailcontent` varchar(300) COLLATE gbk_bin NOT NULL,
  `resFileUrl` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '璧勬簮鏂囦欢',
  PRIMARY KEY (`activityid`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_activitydata` */

DROP TABLE IF EXISTS `t_activitydata`;

CREATE TABLE `t_activitydata` (
  `uidActor` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ActivityID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `bFinished` tinyint(4) DEFAULT '0',
  `bTakeAward` tinyint(4) DEFAULT '0',
  `ActivityProgress` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`uidActor`,`ActivityID`),
  KEY `index_uid_activityid` (`uidActor`,`ActivityID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_actors` */

DROP TABLE IF EXISTS `t_actors`;

CREATE TABLE `t_actors` (
  `UserID` int(10) unsigned NOT NULL,
  `Name` char(18) COLLATE gbk_bin NOT NULL,
  `Level` tinyint(3) unsigned NOT NULL,
  `Spirit` int(10) unsigned NOT NULL,
  `Shield` int(10) unsigned NOT NULL,
  `Avoid` int(10) unsigned NOT NULL,
  `ActorExp` int(10) unsigned NOT NULL,
  `ActorLayer` int(10) unsigned NOT NULL,
  `ActorNimbus` int(10) unsigned NOT NULL,
  `ActorAptitude` int(10) unsigned NOT NULL,
  `ActorSex` tinyint(3) unsigned NOT NULL,
  `uid` bigint(20) unsigned NOT NULL,
  `ActorMoney` int(10) unsigned NOT NULL DEFAULT '0',
  `ActorTicket` int(10) unsigned NOT NULL DEFAULT '0',
  `ActorStone` int(10) unsigned NOT NULL DEFAULT '0',
  `ActorFacade` smallint(5) unsigned NOT NULL,
  `ActorBloodUp` int(10) unsigned NOT NULL DEFAULT '0',
  `nDir` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ptX` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ptY` smallint(5) unsigned NOT NULL DEFAULT '0',
  `uidMaster` bigint(20) unsigned NOT NULL,
  `CityID` smallint(5) unsigned NOT NULL,
  `Honor` int(10) NOT NULL DEFAULT '0',
  `Credit` int(10) NOT NULL DEFAULT '0',
  `ActorNimbusSpeed` int(11) NOT NULL DEFAULT '0',
  `GodSwordNimbus` int(11) NOT NULL DEFAULT '0',
  `UserFlag` binary(2) NOT NULL,
  `VipLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Recharge` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`),
  KEY `name` (`Name`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_allowenter` */

DROP TABLE IF EXISTS `t_allowenter`;

CREATE TABLE `t_allowenter` (
  `UserID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`UserID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_building` */

DROP TABLE IF EXISTS `t_building`;

CREATE TABLE `t_building` (
  `Uid_Building` bigint(20) unsigned NOT NULL,
  `BuildingType` tinyint(3) unsigned NOT NULL,
  `LastTakeResTime` int(10) unsigned NOT NULL DEFAULT '0',
  `Uid_Owner` bigint(20) unsigned NOT NULL,
  `OwnerNoTakeResNum` int(11) NOT NULL DEFAULT '0',
  `LastCollectTime` int(11) unsigned NOT NULL DEFAULT '0',
  `TotalCollectResOfDay` int(11) unsigned NOT NULL DEFAULT '0',
  `bAutoTakeRes` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_Building`),
  KEY `index_uiduser` (`Uid_Owner`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_buildrecord` */

DROP TABLE IF EXISTS `t_buildrecord`;

CREATE TABLE `t_buildrecord` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `BuildType` tinyint(3) unsigned NOT NULL,
  `HappenTime` int(10) unsigned NOT NULL,
  `UserName` char(16) COLLATE gbk_bin NOT NULL,
  `HandleName` char(16) COLLATE gbk_bin NOT NULL,
  `Context` char(50) COLLATE gbk_bin NOT NULL,
  PRIMARY KEY (`Uid_User`,`BuildType`,`HappenTime`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_cdtimer` */

DROP TABLE IF EXISTS `t_cdtimer`;

CREATE TABLE `t_cdtimer` (
  `UidActor` bigint(20) unsigned NOT NULL,
  `CDTimerID` smallint(5) unsigned NOT NULL,
  `EndTime` int(10) unsigned NOT NULL,
  PRIMARY KEY (`UidActor`,`CDTimerID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_challengerank` */

DROP TABLE IF EXISTS `t_challengerank`;

CREATE TABLE `t_challengerank` (
  `Rank` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uidUser` bigint(20) unsigned NOT NULL,
  `Name` char(18) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `layer` int(10) unsigned NOT NULL DEFAULT '0',
  `titleID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `challengeNum` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `viplevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Rank`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_chengjiu` */

DROP TABLE IF EXISTS `t_chengjiu`;

CREATE TABLE `t_chengjiu` (
  `UserUID` bigint(20) unsigned NOT NULL,
  `ChengJiuID` smallint(5) unsigned NOT NULL,
  `CurCount` smallint(5) unsigned NOT NULL,
  `FinishTime` int(10) unsigned NOT NULL,
  PRIMARY KEY (`UserUID`,`ChengJiuID`),
  KEY `index_uidActor_ChengJiuID` (`UserUID`,`ChengJiuID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_chengjiupart` */

DROP TABLE IF EXISTS `t_chengjiupart`;

CREATE TABLE `t_chengjiupart` (
  `UserUID` bigint(20) unsigned NOT NULL,
  `ChengJiuPoint` int(11) NOT NULL DEFAULT '0',
  `GetChengJiuNum` int(11) NOT NULL DEFAULT '0',
  `ActiveTitleID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`UserUID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_combat` */

DROP TABLE IF EXISTS `t_combat`;

CREATE TABLE `t_combat` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `uidLineup1` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup3` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup4` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup5` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup6` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup7` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup8` bigint(20) unsigned NOT NULL DEFAULT '0',
  `uidLineup9` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_doufa` */

DROP TABLE IF EXISTS `t_doufa`;

CREATE TABLE `t_doufa` (
  `UserUid` bigint(20) unsigned NOT NULL,
  `GetHonorToday` int(11) NOT NULL DEFAULT '0',
  `RecordTime` int(10) unsigned NOT NULL DEFAULT '0',
  `JoinChallengeNum` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `LastJoinChallengeTime` int(10) unsigned NOT NULL DEFAULT '0',
  `MaxChallengeNumToday` smallint(5) unsigned NOT NULL DEFAULT '0',
  `DouFaQieCuoEnemy` varbinary(1024) NOT NULL DEFAULT '',
  PRIMARY KEY (`UserUid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_friend` */

DROP TABLE IF EXISTS `t_friend`;

CREATE TABLE `t_friend` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid_User` bigint(20) unsigned NOT NULL,
  `uid_Friend` bigint(20) unsigned NOT NULL,
  `RelationNum` int(10) unsigned NOT NULL,
  `Flag` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `FriendName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `FriendLevel` tinyint(3) unsigned NOT NULL,
  `FriendSex` tinyint(3) unsigned NOT NULL,
  `FriendSynName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `bOneCity` tinyint(1) NOT NULL,
  `FriendFacade` smallint(5) unsigned NOT NULL DEFAULT '0',
  `FriendTitleID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `VipLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  KEY `index_uiduser_uidFriend` (`uid_User`,`uid_Friend`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_friend_event` */

DROP TABLE IF EXISTS `t_friend_event`;

CREATE TABLE `t_friend_event` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid_User` bigint(20) unsigned NOT NULL,
  `uid_Friend` bigint(20) unsigned NOT NULL,
  `AddRelationNum` int(10) unsigned NOT NULL,
  `Descript` varchar(100) COLLATE gbk_bin NOT NULL,
  `Time` bigint(20) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `index_uiduser_uidFriend` (`uid_User`,`uid_Friend`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_friendmsg` */

DROP TABLE IF EXISTS `t_friendmsg`;

CREATE TABLE `t_friendmsg` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid_User` bigint(20) unsigned NOT NULL,
  `uid_SrcUser` bigint(20) unsigned NOT NULL,
  `SrcUserName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `SrcUserLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SrcUserSex` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SrcUserSynName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `bOneCity` tinyint(1) NOT NULL DEFAULT '0',
  `SrcUserFacade` smallint(5) unsigned NOT NULL DEFAULT '0',
  `SrcUserTitleID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  KEY `index_uiduser_uidFriend` (`uid_User`,`uid_SrcUser`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_fubenbasic` */

DROP TABLE IF EXISTS `t_fubenbasic`;

CREATE TABLE `t_fubenbasic` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `LastFreeEnterFuBenTime` int(10) unsigned NOT NULL DEFAULT '0',
  `LastStoneEnterFuBenTime` int(10) unsigned NOT NULL DEFAULT '0',
  `LastSynWelfareEnterFuBenTime` int(10) unsigned NOT NULL DEFAULT '0',
  `FreeEnterFuBenNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  `SynWelfareEnterFuBenNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  `CostStoneEnterFuBenNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  `FuBenNum` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `LastEnterFuBenGodSword` int(10) unsigned NOT NULL DEFAULT '0',
  `EnterSynFuBenNum` smallint(5) NOT NULL DEFAULT '0',
  `LastEnterSynFuBenTime` int(10) NOT NULL DEFAULT '0',
  `EnterGodSwordWorldNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LastVipEnterFuBenTime` int(10) unsigned NOT NULL DEFAULT '0',
  `VipEnterFuBenNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  `FinishedTask` binary(32) NOT NULL DEFAULT '0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_fubenprogress` */

DROP TABLE IF EXISTS `t_fubenprogress`;

CREATE TABLE `t_fubenprogress` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `FuBenID` tinyint(3) unsigned NOT NULL,
  `Level` tinyint(3) unsigned NOT NULL,
  `KillMonsterNum` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`Uid_User`,`FuBenID`),
  KEY `index_uiduser_fuBenID` (`Uid_User`,`FuBenID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_fumodong` */

DROP TABLE IF EXISTS `t_fumodong`;

CREATE TABLE `t_fumodong` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `EndOnHookTime` int(10) unsigned NOT NULL DEFAULT '0',
  `AccelNumOfDay` int(10) unsigned NOT NULL DEFAULT '0',
  `LastAccelTime` int(10) unsigned NOT NULL DEFAULT '0',
  `Level` int(10) unsigned NOT NULL DEFAULT '0',
  `LastGiveExpTime` int(10) unsigned NOT NULL DEFAULT '0',
  `GiveExp` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_gathergodhouse` */

DROP TABLE IF EXISTS `t_gathergodhouse`;

CREATE TABLE `t_gathergodhouse` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `LastFlushEmployTime` int(10) unsigned NOT NULL DEFAULT '0',
  `IdEmployee1` int(10) unsigned NOT NULL DEFAULT '0',
  `IdEmployee2` int(10) unsigned NOT NULL DEFAULT '0',
  `IdEmployee3` int(10) unsigned NOT NULL DEFAULT '0',
  `IdEmployee4` int(10) unsigned NOT NULL DEFAULT '0',
  `LastFlushMagicTime` int(10) unsigned NOT NULL DEFAULT '0',
  `idMagicBook1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `idMagicBook2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `idMagicBook3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `idMagicBook4` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_godstonelog` */

DROP TABLE IF EXISTS `t_godstonelog`;

CREATE TABLE `t_godstonelog` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛id',
  `type` tinyint(3) unsigned DEFAULT NULL COMMENT '绫诲瀷:0鑾峰緱,1娑堣垂',
  `serverid` smallint(6) DEFAULT NULL COMMENT '鏈嶅姟鍣╥d',
  `happentime` datetime DEFAULT NULL COMMENT '鍙戠敓鏃堕棿',
  `vargodstone` int(11) DEFAULT NULL COMMENT '鍙樺姩浠欑煶',
  `aftergodstone` int(11) DEFAULT NULL COMMENT '鍙樺姩鍚庝粰鐭�,
  `description` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '璇存槑',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_godswordshop` */

DROP TABLE IF EXISTS `t_godswordshop`;

CREATE TABLE `t_godswordshop` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `LastFlushTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `IdGodSword1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `IdGodSword2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `IdGodSword3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `IdGodSword4` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_goods` */

DROP TABLE IF EXISTS `t_goods`;

CREATE TABLE `t_goods` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `PositionGoods` tinyint(3) unsigned NOT NULL,
  `Location` smallint(5) NOT NULL DEFAULT '0',
  `UidGoods` bigint(20) unsigned NOT NULL,
  `IDGoods` smallint(5) unsigned NOT NULL,
  `CreateTime` int(10) unsigned NOT NULL,
  `Number` tinyint(3) unsigned NOT NULL,
  `Binded` tinyint(3) NOT NULL,
  `BinData` varbinary(30) NOT NULL,
  PRIMARY KEY (`Uid_User`,`PositionGoods`,`UidGoods`),
  KEY `index_uid_user` (`Uid_User`),
  KEY `index_uiduser_position` (`Uid_User`,`PositionGoods`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_magic` */

DROP TABLE IF EXISTS `t_magic`;

CREATE TABLE `t_magic` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `MagicID` smallint(5) unsigned NOT NULL,
  `Level` tinyint(3) unsigned NOT NULL,
  `MagicPosition` tinyint(4) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`,`MagicID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_mail` */

DROP TABLE IF EXISTS `t_mail`;

CREATE TABLE `t_mail` (
  `MailID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid_User` bigint(20) unsigned NOT NULL,
  `uid_SendUser` bigint(20) unsigned NOT NULL,
  `Name_SendUser` varchar(16) COLLATE gbk_bin NOT NULL,
  `Mail_Type` tinyint(3) unsigned NOT NULL,
  `stone` int(10) unsigned NOT NULL DEFAULT '0',
  `money` int(10) unsigned NOT NULL DEFAULT '0',
  `ticket` int(10) unsigned NOT NULL DEFAULT '0',
  `SynID` int(10) unsigned NOT NULL DEFAULT '0',
  `ThemeText` varchar(50) COLLATE gbk_bin NOT NULL,
  `ContentText` varchar(300) COLLATE gbk_bin NOT NULL,
  `Time` bigint(20) unsigned NOT NULL,
  `bRead` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `GoodsData` varbinary(1024) NOT NULL DEFAULT '',
  PRIMARY KEY (`MailID`),
  KEY `mail_index` (`uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_onlinelog` */

DROP TABLE IF EXISTS `t_onlinelog`;

CREATE TABLE `t_onlinelog` (
  `recordid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id,搴忓垪鍙凤紝',
  `userid` int(10) unsigned NOT NULL COMMENT '鐢ㄦ埛ID',
  `onlineserverid` smallint(5) unsigned DEFAULT NULL COMMENT '鍦ㄧ嚎鏈嶅姟鍣╥d',
  `logintime` datetime DEFAULT NULL COMMENT '涓婄嚎鏃堕棿',
  `loginip` varchar(20) COLLATE gbk_bin DEFAULT NULL COMMENT '涓婄嚎ip',
  `logingodstone` int(11) DEFAULT '0' COMMENT '涓婄嚎鏃朵粰鐭冲�',
  `logoutime` datetime DEFAULT NULL COMMENT '涓嬬嚎鏃堕棿',
  `logoutgodstone` int(11) DEFAULT '0' COMMENT '涓嬬嚎鏃朵粰鐭�,
  PRIMARY KEY (`recordid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_packet` */

DROP TABLE IF EXISTS `t_packet`;

CREATE TABLE `t_packet` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `Capacity` smallint(5) unsigned NOT NULL,
  `GoodsNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_rechargelog` */

DROP TABLE IF EXISTS `t_rechargelog`;

CREATE TABLE `t_rechargelog` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛',
  `rechargetime` datetime DEFAULT NULL COMMENT '鍏呭�鏃堕棿',
  `amount` int(11) DEFAULT NULL COMMENT '鍏呭�閲戝竵',
  `godstone` int(11) DEFAULT NULL COMMENT '鍏呭�鑾峰緱浠欑煶',
  `pregodstone` int(11) DEFAULT NULL COMMENT '鍏呭�鍓嶄粰鐭�,
  `chanel` tinyint(4) DEFAULT NULL COMMENT '鍏呭�娓犻亾:0瀹樻柟,1鏂版氮',
  `rechargeip` varchar(20) COLLATE gbk_bin DEFAULT NULL COMMENT '鍏呭�ip',
  `associateid` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '涓庣涓夋柟鍏宠仈浜嬪姟id',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_serverinfo` */

DROP TABLE IF EXISTS `t_serverinfo`;

CREATE TABLE `t_serverinfo` (
  `serverid` smallint(5) unsigned NOT NULL COMMENT '鏈嶅姟鍣╥d',
  `servername` varchar(18) COLLATE gbk_bin DEFAULT NULL COMMENT '鏈嶅姟鍣ㄥ悕绉�,
  `cooperation` tinyint(3) unsigned DEFAULT NULL COMMENT '鍚堜綔鏂�0鑷繁锛�锛屾柊娴�,
  `serverip` varchar(20) COLLATE gbk_bin DEFAULT NULL COMMENT '鏈嶅姟鍣╥p',
  `serverport` smallint(5) unsigned DEFAULT NULL COMMENT '鏈嶅姟鍣ㄧ鍙�,
  PRIMARY KEY (`serverid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_shopmall` */

DROP TABLE IF EXISTS `t_shopmall`;

CREATE TABLE `t_shopmall` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `goodsid` smallint(5) unsigned NOT NULL COMMENT '鐗╁搧ID',
  `num` smallint(5) unsigned NOT NULL DEFAULT '1' COMMENT '鐗╁搧鏁伴噺',
  `type` tinyint(3) unsigned NOT NULL COMMENT '绫诲瀷: 0:鐑崠,1:鏅�,2:瀹濈煶,3:绀煎嵎鍟嗗簵,4:鐏电煶鍟嗗簵,5:鑽ｈ獕鍟嗗簵',
  `moneytype` tinyint(3) unsigned NOT NULL COMMENT '璐у竵绫诲瀷: 0:浠欑煶,1:绀煎嵎,2:鐏电煶,3:鑽ｈ獕',
  `moneynum` int(11) NOT NULL COMMENT '闇�澶氬皯璐у竵',
  `binded` tinyint(3) unsigned NOT NULL DEFAULT '1' COMMENT '鏄惁缁戝畾',
  `starttime` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT '0:鏃犻檺鍒舵椂闂�闈�:鏈夐檺鍒舵椂闂�,
  `endtime` datetime NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT '0:鏃犻檺鍒舵椂闂�闈�:鏈夐檺鍒舵椂闂�,
  `leftFen` int(11) NOT NULL DEFAULT '-1' COMMENT '褰撳墠鍓╀綑鍙崠浠芥暟,-1琛ㄧず鏃犻檺涓�,
  `goodsname` char(18) COLLATE gbk_bin NOT NULL COMMENT '鐗╁搧鍚嶇О',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=106 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_shopmalllog` */

DROP TABLE IF EXISTS `t_shopmalllog`;

CREATE TABLE `t_shopmalllog` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `userid` int(10) unsigned DEFAULT NULL,
  `shopid` int(10) unsigned DEFAULT NULL,
  `buytime` datetime DEFAULT NULL,
  `buynums` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_status` */

DROP TABLE IF EXISTS `t_status`;

CREATE TABLE `t_status` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `StatusID` smallint(5) unsigned NOT NULL,
  `EndStatusTime` int(10) unsigned NOT NULL,
  `UidCreator` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`Uid_User`,`StatusID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_syndicate` */

DROP TABLE IF EXISTS `t_syndicate`;

CREATE TABLE `t_syndicate` (
  `SynID` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `SynName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `Uid_Leader` bigint(20) unsigned NOT NULL,
  `LeaderName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `Level` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `Exp` int(10) unsigned NOT NULL DEFAULT '0',
  `MemberNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`SynID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_syndicateapplyer` */

DROP TABLE IF EXISTS `t_syndicateapplyer`;

CREATE TABLE `t_syndicateapplyer` (
  `SynID` smallint(5) unsigned NOT NULL,
  `Uid_ApplyUser` bigint(20) unsigned NOT NULL,
  `ApplyUserName` char(16) COLLATE gbk_bin NOT NULL,
  `ApplyUserLevel` tinyint(3) unsigned NOT NULL,
  `ApplyUserLayer` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`SynID`,`Uid_ApplyUser`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_syndicatemagic` */

DROP TABLE IF EXISTS `t_syndicatemagic`;

CREATE TABLE `t_syndicatemagic` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `SynMagicID` smallint(5) unsigned NOT NULL,
  `Level` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`Uid_User`,`SynMagicID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_syndicatemember` */

DROP TABLE IF EXISTS `t_syndicatemember`;

CREATE TABLE `t_syndicatemember` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `SynID` smallint(5) unsigned NOT NULL,
  `UserName` char(16) COLLATE gbk_bin NOT NULL,
  `Userlevel` tinyint(3) unsigned NOT NULL,
  `Position` tinyint(3) unsigned NOT NULL,
  `Contribution` int(10) unsigned NOT NULL,
  `VipLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_talismanworldrecord` */

DROP TABLE IF EXISTS `t_talismanworldrecord`;

CREATE TABLE `t_talismanworldrecord` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `TalismanWorldID` smallint(5) unsigned NOT NULL,
  `EnterNumOfDay` tinyint(3) unsigned NOT NULL,
  `LastEnterTime` int(10) unsigned NOT NULL,
  PRIMARY KEY (`Uid_User`,`TalismanWorldID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_task` */

DROP TABLE IF EXISTS `t_task`;

CREATE TABLE `t_task` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `TaskID` smallint(5) unsigned NOT NULL,
  `FinishTime` int(10) unsigned NOT NULL,
  `CurCount` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`Uid_User`,`TaskID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_taskpart` */

DROP TABLE IF EXISTS `t_taskpart`;

CREATE TABLE `t_taskpart` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `TaskHistory` binary(32) NOT NULL DEFAULT '0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0',
  `LastUpdateTime` int(10) unsigned NOT NULL DEFAULT '0',
  `NotSaveNum` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TaskStatus` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `NotSaveTaskID` varbinary(2048) NOT NULL,
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_test` */

DROP TABLE IF EXISTS `t_test`;

CREATE TABLE `t_test` (
  `index` int(11) NOT NULL AUTO_INCREMENT,
  `uid` bigint(20) unsigned DEFAULT NULL,
  `goodsid` smallint(5) unsigned DEFAULT NULL,
  `CreateTime` int(10) unsigned DEFAULT NULL,
  `Number` tinyint(3) unsigned DEFAULT NULL,
  `Binded` tinyint(3) unsigned DEFAULT NULL,
  `goodsdata` varbinary(30) DEFAULT NULL,
  PRIMARY KEY (`index`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_trade` */

DROP TABLE IF EXISTS `t_trade`;

CREATE TABLE `t_trade` (
  `Uid_Goods` bigint(20) unsigned NOT NULL,
  `Uid_Seller` bigint(20) unsigned NOT NULL,
  `SellerName` char(16) COLLATE gbk_bin NOT NULL,
  `Price` int(10) unsigned NOT NULL,
  `GoodsCategory` tinyint(3) unsigned NOT NULL,
  `SubClass` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `Time` bigint(20) unsigned NOT NULL,
  `GoodsData` varbinary(2048) NOT NULL,
  PRIMARY KEY (`Uid_Goods`),
  KEY `index_uidgoods_uidseller` (`Uid_Goods`,`Uid_Seller`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_traininghall` */

DROP TABLE IF EXISTS `t_traininghall`;

CREATE TABLE `t_traininghall` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `RemainNum` smallint(5) unsigned NOT NULL DEFAULT '0',
  `BeginTime` int(10) unsigned NOT NULL DEFAULT '0',
  `LastFinishTime` int(10) unsigned NOT NULL DEFAULT '0',
  `GetExp` int(10) NOT NULL DEFAULT '0',
  `TrainStatus` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `VipFinishTrainNum` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `LastVipFinishTrainTime` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_user` */

DROP TABLE IF EXISTS `t_user`;

CREATE TABLE `t_user` (
  `UserID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '鐢ㄦ埛ID',
  `UserName` varchar(33) COLLATE gbk_bin NOT NULL COMMENT '鐢ㄦ埛鍚�,
  `Password` char(33) COLLATE gbk_bin DEFAULT NULL COMMENT '瀵嗙爜',
  `channel` timestamp NULL DEFAULT NULL COMMENT '娓犻亾:0璁父,1鏂版氮,2涓夊叚闆�3鑵捐',
  `registertime` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '娉ㄥ唽鏃堕棿',
  `registerip` varchar(20) COLLATE gbk_bin DEFAULT 'NULL' COMMENT '娉ㄥ唽ip',
  `onlinetime` int(11) DEFAULT '0' COMMENT '鍦ㄧ嚎鏃堕暱',
  `lastlogintime` timestamp NULL DEFAULT NULL COMMENT '鏈�悗鐧婚檰鏃堕棿',
  `lastloginip` varchar(20) COLLATE gbk_bin DEFAULT 'NULL' COMMENT '鏈�悗鐧婚檰ip',
  `onlineserverid` smallint(6) DEFAULT '0' COMMENT '鍦ㄧ嚎鏈嶅姟鍣�涓�琛ㄧず涓嶅湪绾�,
  `onlinerecordid` int(10) unsigned DEFAULT '0' COMMENT '鍦ㄧ嚎璁板綍ID',
  PRIMARY KEY (`UserID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin CHECKSUM=1 DELAY_KEY_WRITE=1 ROW_FORMAT=DYNAMIC;

/*Table structure for table `t_userrank` */

DROP TABLE IF EXISTS `t_userrank`;

CREATE TABLE `t_userrank` (
  `Rank` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '鎺掑悕',
  `uidUser` bigint(20) unsigned NOT NULL COMMENT '鐜╁UID',
  `Name` char(18) COLLATE gbk_bin NOT NULL,
  `Level` tinyint(3) unsigned NOT NULL,
  `Exp` int(10) unsigned NOT NULL,
  `Layer` int(10) unsigned NOT NULL,
  `viplevel` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`Rank`),
  KEY `index_uid` (`uidUser`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_xiulian` */

DROP TABLE IF EXISTS `t_xiulian`;

CREATE TABLE `t_xiulian` (
  `Uid_User` bigint(20) unsigned NOT NULL,
  `AloneXiuLianState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `AloneHours` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `AloneLastGetNimbusTime` int(10) unsigned NOT NULL DEFAULT '0',
  `AloneEndTime` int(10) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor3` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor4` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor5` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor6` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor7` bigint(20) unsigned NOT NULL DEFAULT '0',
  `AloneXLActor8` bigint(20) unsigned NOT NULL DEFAULT '0',
  `GetGodSwordNimbus` int(10) unsigned NOT NULL DEFAULT '0',
  `AloneXLGetNimbus` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus2` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus3` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus4` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus5` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus6` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus7` int(11) NOT NULL DEFAULT '0',
  `AloneXLGetNimbus8` int(11) NOT NULL DEFAULT '0',
  `TwoXiuLianState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TwoXLHours` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TwoXLLastGetNimbusTime` int(10) unsigned NOT NULL DEFAULT '0',
  `TwoXLTotalNimbus` int(11) NOT NULL DEFAULT '0',
  `TwoXLEndTime` int(10) unsigned NOT NULL DEFAULT '0',
  `TwoXLUidActor` bigint(20) unsigned NOT NULL,
  `TwoXLUidFriend` bigint(20) unsigned NOT NULL,
  `FriendFacade` int(10) unsigned NOT NULL DEFAULT '0',
  `FriendName` char(18) COLLATE gbk_bin NOT NULL DEFAULT '""',
  `MagicXLSeq` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Uid_User`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_xiulian_record` */

DROP TABLE IF EXISTS `t_xiulian_record`;

CREATE TABLE `t_xiulian_record` (
  `XiuLianType` tinyint(3) unsigned NOT NULL,
  `XiuLianState` tinyint(3) unsigned NOT NULL,
  `Hours` tinyint(3) unsigned NOT NULL,
  `Mode` tinyint(3) unsigned NOT NULL,
  `LastGetNimbusTime` int(10) unsigned NOT NULL DEFAULT '0',
  `TotalNimbus` int(11) NOT NULL DEFAULT '0',
  `EndTime` int(10) unsigned NOT NULL DEFAULT '0',
  `AskSeq` int(10) unsigned NOT NULL DEFAULT '0',
  `UidSource` bigint(20) unsigned NOT NULL,
  `UidSourceActor` bigint(20) unsigned NOT NULL,
  `SourceName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '',
  `UidFriend` bigint(20) unsigned NOT NULL,
  `UidFriendActor` bigint(20) unsigned NOT NULL,
  `FriendName` char(16) COLLATE gbk_bin NOT NULL DEFAULT '',
  `MagicID` smallint(5) unsigned NOT NULL,
  `AskTime` int(10) unsigned NOT NULL DEFAULT '0',
  `FriendTotalNimbus` int(10) unsigned NOT NULL DEFAULT '0',
  `SourceNotTakeNimbus` int(10) unsigned NOT NULL DEFAULT '0',
  `FriendNotTakeNimbus` int(10) unsigned NOT NULL DEFAULT '0',
  `bStudyMagic` tinyint(3) DEFAULT '0',
  `FriendLastGetNimbusTime` int(10) unsigned NOT NULL DEFAULT '0',
  `SourceNimbusSpeed` int(10) unsigned NOT NULL DEFAULT '0',
  `FriendNimbusSpeed` int(10) unsigned NOT NULL DEFAULT '0',
  `SourceLayer` int(10) unsigned NOT NULL DEFAULT '0',
  `FriendLayer` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `trygoods` */

DROP TABLE IF EXISTS `trygoods`;

CREATE TABLE `trygoods` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `lowUid` int(10) unsigned NOT NULL DEFAULT '0',
  `highUid` int(10) unsigned NOT NULL DEFAULT '0',
  `uidGoods` bigint(20) unsigned NOT NULL DEFAULT '0',
  `goodsID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `createTime` int(10) unsigned NOT NULL DEFAULT '0',
  `number` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `binded` tinyint(1) NOT NULL DEFAULT '0',
  `packetGoodsData` binary(14) NOT NULL DEFAULT '""\0\0\0\0\0\0\0\0\0\0\0\0',
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/* Function  structure for function  `Func_Update_BuildRecordInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_BuildRecordInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_BuildRecordInfo`(pUid_User bigint unsigned,pRecordNum int unsigned,pBuildRecord blob(10240)) RETURNS tinyint(4)
BEGIN
	declare t_buildtype tinyint unsigned default 0;
	declare t_time int unsigned default 0;
	declare t_username varchar(18) default '';
	declare t_handlename varchar(18) default '';
	declare t_context varchar(18) default '';
	declare t_count int unsigned default 0;
	declare t_pos	int unsigned default 1;
	declare t_size  int unsigned default 0;
	set t_size = ToInt(SUBSTRING(pBuildRecord,t_pos,4));
	set t_pos = t_pos + 4;
	while t_count < pRecordNum do
		set t_buildtype = ASCII(SUBSTRING(pBuildRecord,t_pos,1));
		set t_pos = t_pos + 1;
		set t_time = ToUInt(SUBSTRING(pBuildRecord,t_pos,4));
		set t_pos = t_pos + 4;
		set t_username = ToString(SUBSTRING(pBuildRecord,t_pos,18));
		set t_pos = t_pos + 18;
		
		set t_handlename = ToString(SUBSTRING(pBuildRecord,t_pos,18));
		set t_pos = t_pos + 18;
		set t_context = ToString(SUBSTRING(pBuildRecord,t_pos,50));
		set t_pos = t_pos + 50;
		
		insert into t_buildrecord values(pUid_User, t_buildtype, t_time, t_username, t_handlename, t_context);
		set t_count =  t_count + 1;
	end while;
	return 1;
    END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_EquipGoodsInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_EquipGoodsInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_EquipGoodsInfo`(pUid_User bigint unsigned,pEquipNum smallint unsigned,pUnionMaxSize tinyint unsigned,pEquipGoodsData blob(10240)) RETURNS tinyint(1)
BEGIN
	declare t_uidGoods bigint unsigned default 0;
	declare t_goodsID  smallint unsigned default 0;
	declare t_goodsTest smallint unsigned default 0;
	declare t_createTime int unsigned default 0;
	declare t_number  tinyint unsigned default 0;
	declare t_binded  bool default false;
	declare t_EquipGoodsData varbinary(14) default "";
	declare t_count int unsigned default 0;
	declare t_pos	int unsigned default 1;
	while t_count < pEquipNum do
	      -- t_uidGoods
		set t_uidGoods = ToUBigInt(SUBSTRING(pEquipGoodsData,t_pos,8));
		set t_pos = t_pos + 8;
	      -- t_goodsID
		set t_goodsID = ToUSmallInt(SUBSTRING(pEquipGoodsData,t_pos,2));
		set t_pos = t_pos + 2;
	      -- t_createTime
		set t_createTime = ToUInt(SUBSTRING(pEquipGoodsData,t_pos,4));
		set t_pos = t_pos + 4;
	      -- t_number
		set t_number = ASCII(SUBSTRING(pEquipGoodsData,t_pos,1));
		set t_pos = t_pos + 1;
	      -- t_binded
		set t_binded = ASCII(SUBSTRING(pEquipGoodsData,t_pos,1));
		set t_pos = t_pos + 1;
	      -- t_EquipGoodsData
		set t_EquipGoodsData = SUBSTRING(pEquipGoodsData,t_pos,14);
		set t_pos = t_pos + 14;
		
	      set t_goodsTest = 0;
	      select IDGoods into t_goodsTest from t_goods where UidGoods = t_uidGoods and Uid_User=pUid_User;
	      if t_goodsTest = 0 then
			insert into t_goods values(pUid_User,1,t_uidGoods,t_goodsID,t_createTime,t_number,t_binded,t_EquipGoodsData);
	      else
			-- 閺囧瓨鏌婇弫鐗堝祦
			update t_goods set Uid_User = pUid_User, PositionGoods = 1, IDGoods = t_goodsID,CreateTime = t_createTime,Number=t_number,Binded = t_binded,BinData = t_EquipGoodsData  where UidGoods = t_uidGoods;
	      end if;
	      set t_count = t_count + 1;
      end while;
	
      return true;
    END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_GoodsInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_GoodsInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_GoodsInfo`(pContainerTYpe tinyint,pUid_User bigint unsigned,pGoodsNum smallint unsigned,pPacketGoodsData blob(10240)) RETURNS tinyint(1)
BEGIN
	declare t_uidGoods bigint unsigned default 0;
	declare t_goodsID  smallint unsigned default 0;
	declare t_goodsTest smallint unsigned default 0;
	declare t_createTime int unsigned default 0;
	declare t_number  tinyint unsigned default 0;
	declare t_binded  bool default false;
	declare t_packetGoodsData varbinary(14) default "";
	declare t_count int unsigned default 0;
	declare t_pos	int unsigned default 1;
        declare t_loc smallint default 0;
	
	while t_count < pGoodsNum do
              -- location
               set t_loc =  ToUSmallInt(SUBSTRING(pPacketGoodsData,t_pos,2));
		set t_pos = t_pos + 2; 
	      -- t_uidGoods
		set t_uidGoods = ToUBigInt(SUBSTRING(pPacketGoodsData,t_pos,8));
		set t_pos = t_pos + 8;
	      -- t_goodsID
		set t_goodsID = ToUSmallInt(SUBSTRING(pPacketGoodsData,t_pos,2));
		set t_pos = t_pos + 2; 
	      -- t_createTime
		set t_createTime = ToUInt(SUBSTRING(pPacketGoodsData,t_pos,4));
		set t_pos = t_pos + 4;
	      -- t_number
		set t_number = ASCII(SUBSTRING(pPacketGoodsData,t_pos,1));
		set t_pos = t_pos + 1;
	      -- t_binded
		set t_binded = ASCII(SUBSTRING(pPacketGoodsData,t_pos,1));
		set t_pos = t_pos + 1;
	      -- t_packetGoodsData
		set t_packetGoodsData = SUBSTRING(pPacketGoodsData,t_pos,14);
		set t_pos = t_pos + 14;
		
	      set t_goodsTest = 0;
	      select IDGoods into t_goodsTest from t_goods where UidGoods = t_uidGoods and Uid_User = pUid_User;
	      if t_goodsTest = 0 then
			-- 娑撳秴鐡ㄩ崷銊ф畱閹绘帒鍙�
			insert into t_goods values(pUid_User,pContainerTYpe,t_loc,t_uidGoods,t_goodsID,t_createTime,t_number,t_binded,t_packetGoodsData);
	      else
			-- 閺囧瓨鏌婇弫鐗堝祦
			update t_goods set Uid_User = pUid_User, PositionGoods = pContainerTYpe, location = t_loc,IDGoods = t_goodsID,CreateTime = t_createTime,Number=t_number,Binded = t_binded,BinData = t_packetGoodsData  where UidGoods = t_uidGoods;
	      end if;
	      set t_count = t_count + 1;
      end while;
	
      return true;
		
  END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_MagicPanelInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_MagicPanelInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_MagicPanelInfo`(pUid_User bigint unsigned, pMagicPanelData blob(10240)) RETURNS tinyint(1)
BEGIN
	declare t_countMagic int unsigned default 0;
	declare t_magicNum   smallint unsigned default 0;	-- 瀹歌尙绮＄�锔跨瘎鏉╁洨娈戝▔鏇熸钩閺佷即鍣�
	declare t_magicID    smallint unsigned default 0;
	declare t_magicTest  smallint unsigned default 0;
	declare t_level	     tinyint  unsigned default 0;
	declare t_Position   tinyint  unsigned default 0;
	declare t_pos	     int unsigned default 1;
	
	set t_magicNum = ToUSmallInt(SUBSTRING(pMagicPanelData,t_pos,2));
	set t_pos = t_pos + 2;
	while t_countMagic < t_magicNum do
		-- t_magicID
		set t_magicID = ToUSmallInt(SUBSTRING(pMagicPanelData,t_pos,2));
		set t_pos = t_pos + 2;
		-- t_level
		set t_level = ASCII(SUBSTRING(pMagicPanelData,t_pos,1));
		set t_pos = t_pos + 1;
		-- t_Position
		set t_Position = ASCII(SUBSTRING(pMagicPanelData,t_pos,1));
		set t_pos = t_pos + 1;
		
		set t_magicTest = 0;
		select MagicID into t_magicTest from t_magic where MagicID = t_magicID and Uid_User=pUid_User;
		if t_magicTest = 0 then
			insert into t_magic values(pUid_User,t_magicID,t_level,t_Position);
		else
			update t_magic set Level=t_level ,MagicPosition=t_Position where Uid_User=pUid_User and MagicID=t_magicID;
		end if;
		set t_countMagic = t_countMagic + 1;
	end while;
	return true;
    END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_PacketGoodsInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_PacketGoodsInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_PacketGoodsInfo`(pUid_User bigint unsigned,pGoodsNum smallint unsigned,pUnionMaxSize tinyint unsigned,pPacketGoodsData blob(10240)) RETURNS tinyint(1)
BEGIN
	declare t_uidGoods bigint unsigned default 0;
	declare t_goodsID  smallint unsigned default 0;
	declare t_goodsTest smallint unsigned default 0;
	declare t_createTime int unsigned default 0;
	declare t_number  tinyint unsigned default 0;
	declare t_binded  bool default false;
	declare t_packetGoodsData varbinary(14) default "";
	declare t_count int unsigned default 0;
	declare t_pos	int unsigned default 1;
	
	while t_count < pGoodsNum do
	      -- t_uidGoods
		set t_uidGoods = ToUBigInt(SUBSTRING(pPacketGoodsData,t_pos,8));
		set t_pos = t_pos + 8;
	      -- t_goodsID
		set t_goodsID = ToUSmallInt(SUBSTRING(pPacketGoodsData,t_pos,2));
		set t_pos = t_pos + 2; 
	      -- t_createTime
		set t_createTime = ToUInt(SUBSTRING(pPacketGoodsData,t_pos,4));
		set t_pos = t_pos + 4;
	      -- t_number
		set t_number = ASCII(SUBSTRING(pPacketGoodsData,t_pos,1));
		set t_pos = t_pos + 1;
	      -- t_binded
		set t_binded = ASCII(SUBSTRING(pPacketGoodsData,t_pos,1));
		set t_pos = t_pos + 1;
	      -- t_packetGoodsData
		set t_packetGoodsData = SUBSTRING(pPacketGoodsData,t_pos,14);
		set t_pos = t_pos + 14;
		
	      set t_goodsTest = 0;
	      select IDGoods into t_goodsTest from t_goods where UidGoods = t_uidGoods and Uid_User = pUid_User;
	      if t_goodsTest = 0 then
			-- 娑撳秴鐡ㄩ崷銊ф畱閹绘帒鍙�
			insert into t_goods values(pUid_User,0,t_uidGoods,t_goodsID,t_createTime,t_number,t_binded,t_packetGoodsData);
	      else
			-- 閺囧瓨鏌婇弫鐗堝祦
			update t_goods set Uid_User = pUid_User, PositionGoods = 0, IDGoods = t_goodsID,CreateTime = t_createTime,Number=t_number,Binded = t_binded,BinData = t_packetGoodsData  where UidGoods = t_uidGoods;
	      end if;
	      set t_count = t_count + 1;
      end while;
	
      return true;
		
  END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_StatusInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_StatusInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_StatusInfo`(pUid_User bigint unsigned, pStatusNum tinyint unsigned, pStatusInfo blob(10240)) RETURNS tinyint(4)
BEGIN
	declare t_statusID smallint unsigned default 0;
	declare t_EndStatusTime int unsigned default 0;
	declare t_UidCreate bigint unsigned default 0;
	declare t_count int unsigned default 0;
	declare t_pos	int unsigned default 1;
	declare t_size  int default 0;
	set t_size = ToInt(SUBSTRING(pBuildRecord,t_pos,4));
	set t_pos = t_pos + 4;
	while t_count < pStatusNum do
		set t_statusID = ToSmallInt(SUBSTRING(pBuildRecord,t_pos,2));
		set t_pos = t_pos + 2;
		set t_EndStatusTime = ToUInt(SUBSTRING(pBuildRecord,t_pos,4));
		set t_pos = t_pos + 4;
		set t_UidCreate = ToUBigInt(SUBSTRING(pBuildRecord,t_pos,4));
		set t_pos = t_pos + 8;
		insert into t_status values(pUid_User, t_statusID, t_EndStatusTime, t_UidCreate);
		set t_count = t_count + 1;
	end while;
	return true;
    END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_SynMagicInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_SynMagicInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_SynMagicInfo`(pUid_User bigint unsigned, pSynMagicData blob(10240)) RETURNS tinyint(1)
BEGIN
	declare t_magicNum tinyint unsigned default 0;
	declare t_magicID  smallint unsigned default 0;
	declare t_magicTest smallint unsigned default 0;
	declare t_level tinyint unsigned default 0;
	declare t_countMagic int unsigned default 0;
	declare t_pos int unsigned default 1;
	set t_magicNum = ASCII(SUBSTRING(pSynMagicData,t_pos,1));
	set t_pos = t_pos + 1;
	while t_countMagic < t_magicNum do
		-- t_magicID
		set t_magicID = ToUSmallInt(SUBSTRING(pSynMagicData,t_pos,2));
		set t_pos = t_pos + 2;
		-- t_level
		set t_level = ASCII(SUBSTRING(pSynMagicData,t_pos,1));
		set t_pos = t_pos + 1;
		set t_magicTest = 0;
		select SynMagicID into t_magicTest from t_syndicatemagic where Uid_User=pUid_User and SynMagicID=t_magicID;
		if t_magicTest = 0 then
			insert into t_syndicatemagic values(pUid_User,t_magicID,t_level);
		else
			update t_syndicatemagic set Level = t_level where Uid_User = pUid_User and SynMagicID = t_magicID;
		end if;
		set t_countMagic = t_countMagic + 1;
	end while;
	return true;
    END */$$
DELIMITER ;

/* Function  structure for function  `Func_Update_TaskInfo` */

/*!50003 DROP FUNCTION IF EXISTS `Func_Update_TaskInfo` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `Func_Update_TaskInfo`(pUid_User bigint unsigned, pTask_Num tinyint unsigned, pTaskData blob(10240)) RETURNS tinyint(1)
BEGIN
	declare t_TaskID smallint unsigned default 0;
	declare t_bFinished  bool  default false;
	declare t_CurCount bigint unsigned default 0;
	declare t_Count int default 0;
	declare t_pos	int unsigned default 1;
	while t_Count < pTask_Num do
		-- t_TaskID
		set t_TaskID = ToUSmallInt(SUBSTRING(pTaskData,t_pos,2));
		set t_pos = t_pos + 2;
		-- t_FinishTime
		set t_bFinished = ToUInt(SUBSTRING(pTaskData,t_pos,4));
		set t_pos = t_pos + 4;
		-- t_TaskCount
		set t_CurCount = ToBigInt(SUBSTRING(pTaskData,t_pos,8));
		set t_pos = t_pos + 8;
		Insert into t_task values(pUid_User, t_TaskID, t_bFinished, t_CurCount);
		
		set t_Count = t_Count + 1;
	end while;
	return 1;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToBigInt` */

/*!50003 DROP FUNCTION IF EXISTS `ToBigInt` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToBigInt`(ptr varbinary (8)) RETURNS bigint(20)
BEGIN
        declare  result bigint default 0;
        set result = ASCII(SUBSTRING(ptr,8,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,7,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,6,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,5,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,4,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,3,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,2,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,1,1));
        return result;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToInt` */

/*!50003 DROP FUNCTION IF EXISTS `ToInt` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToInt`(ptr varbinary (4)) RETURNS int(11)
BEGIN
        declare  result int default 0;
        set result = ASCII(SUBSTRING(ptr,4,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,3,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,2,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,1,1));
        return result;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToSmallInt` */

/*!50003 DROP FUNCTION IF EXISTS `ToSmallInt` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToSmallInt`(ptr varbinary (2)) RETURNS smallint(6)
BEGIN
        declare  result smallint default 0;
        set result = ASCII(SUBSTRING(ptr,2,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,1,1));
          return result;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToString` */

/*!50003 DROP FUNCTION IF EXISTS `ToString` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToString`(ptr varbinary (256), pLen int) RETURNS varchar(256) CHARSET gbk COLLATE gbk_bin
BEGIN
	declare t_varchar varchar(256) default '';
	declare t_char char default '';
	declare t_count int default 0;
	declare t_pos int default 1;
	while t_count < pLen do
		set t_char = CHAR(SUBSTRING(ptr,t_pos,1));
		set t_varchar = concat(t_varchar,t_char);
		set t_pos   = t_pos + 1;
		set t_count = t_count + 1;
	end while;
	return t_varchar;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToUBigInt` */

/*!50003 DROP FUNCTION IF EXISTS `ToUBigInt` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToUBigInt`(ptr varbinary (8)) RETURNS bigint(20) unsigned
BEGIN
        declare  result bigint unsigned default 0;
        set result = ASCII(SUBSTRING(ptr,8,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,7,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,6,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,5,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,4,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,3,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,2,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,1,1));
        return result;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToUInt` */

/*!50003 DROP FUNCTION IF EXISTS `ToUInt` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToUInt`(ptr varbinary (4)) RETURNS int(10) unsigned
BEGIN
        declare  result int unsigned default 0;
        set result = ASCII(SUBSTRING(ptr,4,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,3,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,2,1));
        set result = result * 256 +ASCII(SUBSTRING(ptr,1,1));
        return result;
    END */$$
DELIMITER ;

/* Function  structure for function  `ToUSmallInt` */

/*!50003 DROP FUNCTION IF EXISTS `ToUSmallInt` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` FUNCTION `ToUSmallInt`(ptr varbinary (2)) RETURNS smallint(5) unsigned
BEGIN
         declare  result smallint unsigned default 0;
        set result = ASCII(SUBSTRING(ptr,2,1));
        set result = result * 256 + ASCII(SUBSTRING(ptr,1,1));
          return result;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_MyRank_Challenge` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_MyRank_Challenge` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_MyRank_Challenge`(pUidUser bigint unsigned)
BEGIN
	declare t_rank int unsigned default 0;
	select Rank into t_rank from t_challengerank where uidUser = pUidUser;
	if t_rank = 0 then
		select 2;
		select 0;
	else
		select 0;
		select t_rank;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Change_ChallengeRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Change_ChallengeRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Change_ChallengeRank`(pUidUser1 bigint unsigned , pUidUser2 bigint unsigned)
BEGIN
	declare t_rank1 int unsigned default 0;
	declare t_rank2 int unsigned default 0;
        START TRANSACTION; 
	select Rank into t_rank1 from t_challengerank where uidUser=pUidUser1 ;
	select Rank into t_rank2 from t_challengerank where uidUser=pUidUser2 ;
	-- 閬垮厤涓婚敭鍐茬獊
	update t_challengerank set Rank=0 where uidUser = pUidUser1;
	update t_challengerank set Rank=t_rank1 where uidUser = pUidUser2;
	update t_challengerank set Rank=t_rank2 where uidUser = pUidUser1;
         commit;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Check_CanChallenge` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Check_CanChallenge` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Check_CanChallenge`(puidMe bigint  unsigned, puidEnemy bigint unsigned, pNum int)
BEGIN
	declare t_RankMe int unsigned default 0;
	declare t_RankEnemy int unsigned default 0;
	if t_RankEnemy >  t_RankMe then
		select 2;
	else
		if t_RankMe - t_RankEnemy > pNum then
			select 2;
		else
			select 0;
		end if;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteEquip` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteEquip` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_DeleteEquip`(pUid_User bigint unsigned, pUid_Goods bigint unsigned, pLocation tinyint unsigned)
BEGIN
	delete from t_goods where UidGoods = pUid_Goods and Uid_User = pUid_User and PositionGoods = 1 and Location = pLocation ;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteFriendDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteFriendDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_DeleteFriendDataInfo`(pUid_User bigint unsigned, pUid_Friend bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	delete from t_friend where uid_User = pUid_User and uid_Friend = pUid_Friend;
	select uid_User into t_uidUser from t_friend where uid_User = pUid_Friend and uid_Friend = pUid_User;
	if t_uidUser <> 0 then
		update t_friend set Flag = 0 where uid_User = pUid_Friend and uid_Friend = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteFriendEnventDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteFriendEnventDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_DeleteFriendEnventDataInfo`(pTime bigint unsigned)
BEGIN
	delete from t_friend_event where (Time - pTime) >= 86400;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteFriendMsgDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteFriendMsgDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_DeleteFriendMsgDataInfo`(pUid_User bigint unsigned, pUid_SrcUser bigint unsigned)
BEGIN
	delete from t_friendmsg where uid_User = pUid_User and uid_SrcUser = pUid_SrcUser;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeletePacketGoods` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeletePacketGoods` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_DeletePacketGoods`(puidUser bigint unsigned , puidGoods bigint unsigned)
BEGIN
	delete from t_goods where Uid_User = puidUser and PositionGoods = 0 and UidGoods = puidGoods;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteSyndicateApplyInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteSyndicateApplyInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_DeleteSyndicateApplyInfo`(pSynID smallint unsigned,puid_ApplyUser bigint unsigned)
BEGIN
	delete from t_syndicateapplyer where SynID = pSynID and Uid_ApplyUser = puid_ApplyUser; 
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteSyndicateMember` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteSyndicateMember` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_DeleteSyndicateMember`(pSynID smallint unsigned,puid_QuitUser bigint unsigned)
BEGIN
	delete from t_syndicatemember where SynID=pSynID and Uid_User=puid_QuitUser;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_DeleteTradeDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_DeleteTradeDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_DeleteTradeDataInfo`(pUid_Seller bigint unsigned, pUid_Goods bigint unsigned)
BEGIN
	delete from t_trade where Uid_Seller = pUid_Seller and Uid_Goods = pUid_Goods;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_Actor` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_Actor` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Delete_Actor`(pUser_Uid bigint unsigned)
BEGIN
	delete from t_actors where uid = pUser_Uid;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_AllStatus` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_AllStatus` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Delete_AllStatus`(pUidUser bigint unsigned)
BEGIN
	delete from t_status where Uid_User=pUidUser;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_CDTimer` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_CDTimer` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Delete_CDTimer`(pUidActor bigint unsigned)
BEGIN
	delete from t_cdtimer where UidActor = pUidActor;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_Goods` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_Goods` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Delete_Goods`(pUid_User bigint unsigned, pUid_Goods bigint unsigned, pPos tinyint unsigned)
BEGIN
	delete from t_goods where Uid_User = pUid_User and PositionGoods = pPos and UidGoods = pUid_Goods;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_MailData_Req` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_MailData_Req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Delete_MailData_Req`(pMailID int unsigned)
BEGIN
	delete from t_mail where MailID = pMailID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_ResBuildRecord` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_ResBuildRecord` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Delete_ResBuildRecord`(pUidUser bigint unsigned, pBuildingType tinyint unsigned, pHappenTime int unsigned)
BEGIN
	delete from t_buildrecord where Uid_User = pUidUser and BuildType = pBuildingType and HappenTime = pHappenTime;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_Syndicate` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_Syndicate` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Delete_Syndicate`(pSynID smallint unsigned)
BEGIN
	delete from t_syndicate where SynID=pSynID;
	delete from t_syndicateapplyer where SynID = pSynID;
	delete from t_syndicatemember where SynID = pSynID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Delete_XiuLianRecord` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Delete_XiuLianRecord` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Delete_XiuLianRecord`(pAskSeq int unsigned)
BEGIN
	declare t_askSeq int unsigned default 0; 
	select AskSeq into t_askSeq from t_xiulian_record where t_xiulian_record.AskSeq = pAskSeq;
	if t_askSeq <> 0 then
		delete from t_xiulian_record where AskSeq = pAskSeq;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Flush_DouFaEnemy` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Flush_DouFaEnemy` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Flush_DouFaEnemy`(pUidUser bigint unsigned, pEnemyLevel1 tinyint unsigned, pEnemyLevel2 tinyint unsigned,pEnemyLevel3 tinyint unsigned)
BEGIN
	declare t_uid1 bigint unsigned default 0;
	declare t_name1 char(18) default "";
	declare t_facade1 smallint unsigned default 0;
	declare t_viplevel1 tinyint unsigned default 0;
	declare t_uid2 bigint unsigned default 0;
	declare t_name2 char(18) default "";
	declare t_facade2 smallint unsigned default 0;
	declare t_viplevel2 tinyint unsigned default 0;
	declare t_uid3 bigint unsigned default 0;
	declare t_name3 char(18) default "";
	declare t_facade3 smallint unsigned default 0;
	declare t_viplevel3 tinyint unsigned default 0;
	select uid,Name,ActorFacade,VipLevel into t_uid1,t_name1,t_facade1,t_viplevel1 from t_actors where Level = pEnemyLevel1 and uidMaster = 0 and uid <> pUidUser order by rand() limit 0, 1;
	select uid,Name,ActorFacade,VipLevel into t_uid2,t_name2,t_facade2,t_viplevel2 from t_actors where Level = pEnemyLevel2 and uidMaster = 0 and uid <> pUidUser order by rand() limit 0, 1;
	select uid,Name,ActorFacade,VipLevel into t_uid3,t_name3,t_facade3,t_viplevel3 from t_actors where Level = pEnemyLevel3 and uidMaster = 0 and uid <> pUidUser order by rand() limit 0, 1;
	select 0;
	select t_uid1,t_name1,t_facade1,t_viplevel1,t_uid2,t_name2,t_facade2,t_viplevel2,t_uid3,t_name3,t_facade3,t_viplevel3;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Flush_QieCuoEnemy` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Flush_QieCuoEnemy` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Flush_QieCuoEnemy`(pUidUser bigint unsigned,pBeginLevel tinyint unsigned, pEndLevel tinyint unsigned, pNum smallint)
BEGIN
	declare t_userid int default 0;
	select UserID into t_userid from t_actors where Level >= pBeginLevel and Level <= pEndLevel and uidMaster = 0 and uid <> pUidUser limit 0,1;
	if t_userid = 0 then
		select 2;
		select 0,"",0,0,0,0;
	else
		select 0;
		select uid,Name,Level,ActorLayer,ActiveTitleID,VipLevel from t_actors, t_chengjiupart where t_actors.uid = t_chengjiupart.UserUID and
			Level >= pBeginLevel and Level <= pEndLevel and uidMaster = 0 and uid <> pUidUser order by rand() limit 0,pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetActorBasicDataByUIDInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetActorBasicDataByUIDInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_GetActorBasicDataByUIDInfo`(pUserUid bigint unsigned)
BEGIN
          declare t_uid bigint unsigned default 0;
          select uid into t_uid from t_actors where uid = pUserUid;
          if t_uid = 0 then
                    select 2;  -- 閻熸瑦甯熸竟濠冪▔瀹ュ懐鎽犻柛? 
		    select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
          else
                    select 0;   -- 闁瑰瓨鍔曟慨?
		    select * from t_actors where uid = pUserUid;
          end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetActorBasicInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetActorBasicInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetActorBasicInfo`(pUserID int unsigned)
BEGIN
          declare t_userid int unsigned default 0;
 select UserID into t_userid from t_allowenter where UserID = pUserID or UserID=0xffffffff limit 0,1;
      if t_userid = 0 then
          select 6;
          select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
       else
	  set t_userid = 0;
          select UserID into t_userid from t_actors where UserID = pUserID;
          if t_userid = 0 then
                    select 2;
		    select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
          else
                    select 0;
		    select * from t_actors where UserID = pUserID;
          end if;
       end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetActorFacade` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetActorFacade` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_GetActorFacade`(pActorUID bigint unsigned)
BEGIN
	declare t_uid bigint unsigned default 0;
	select uid into t_uid from t_actors where uid = pActorUID;
	if t_uid = 0 then
		select 2;
		select 0,0;
	else
		select 0;
		select uid,ActorFacade from t_actors where uid = pActorUID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetActorIDByNameInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetActorIDByNameInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetActorIDByNameInfo`(pName varchar(18))
BEGIN
     
	declare t_userid int unsigned default 0;
	declare t_uidUser bigint unsigned default 0;
	select UserID,uid into t_userid,t_uidUser from t_actors where Name = pName and t_actors.uidMaster = 0;
	if t_userid = 0 then
		select 2;
		select 0;
	else
		select 0;
		select t_uidUser;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetActorInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetActorInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetActorInfo`(pUserID int unsigned)
BEGIN
      declare t_userid int unsigned default 0;
      declare t_actordata blob(65535);
      select UserID into t_userid from t_allowenter where UserID = pUserID or UserID=0xffffffff limit 0,1;
      if t_userid = 0 then
          select 6;
      else
        set t_userid = 0;
        select UserID, ActorData into t_userid,t_actordata from t_actor where UserID = pUserID;
        if t_userid = 0 then
          select 2;    
        else
          select 0;   
        end if;
      end if;
      select t_userid,t_actordata;
      
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetBuildingInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetBuildingInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetBuildingInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_num int unsigned default 0;
	
        select count(*) into t_num from t_building where Uid_Owner=pUid_User;
        select t_num;
	if t_num = 0 then
		select 0,0,0,0,0,0,0,0;
	else
		select * from t_building where Uid_Owner=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetBuildingRecordInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetBuildingRecordInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetBuildingRecordInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_buildrecord where Uid_User = pUid_User limit 0,1;
	if t_uidUser = 0 then
		select 2;
		select 0,0,'','','';
	else
		select 0;
		select BuildType, HappenTime, UserName, HandleName, Context from t_buildrecord where Uid_User = pUid_User;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetDouFaInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetDouFaInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_GetDouFaInfo`(pUserUid bigint unsigned)
BEGIN
	declare t_UserUid bigint default 0;
	select UserUid into t_UserUid from t_doufa where UserUid = pUserUid;
	if t_UserUid = 0 then
		select 2;
		select pUserUid, 0, 0,0,0,0,0;
	else
		select 0;
		select * from t_doufa where UserUid = pUserUid;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetDouFaRand5Enemy` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetDouFaRand5Enemy` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_GetDouFaRand5Enemy`(pUserUid bigint unsigned)
BEGIN
	declare t_Credit int default 0;
	
	select Credit into t_Credit from t_actors where uid = pUserUid;
	
	select 3;
	select uid from t_actors where Credit < (t_Credit + 500) and Credit > (t_Credit - 500) and UserID <> 0 order by rand() limit 0,3;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetEmployeeInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetEmployeeInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetEmployeeInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_num tinyint unsigned default 0;
	select count(*) into t_num from t_actors where uidMaster = pUid_User;
	if t_num = 0 then
		select 0;
		select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
	else
		select t_num;
		select * from t_actors where uidMaster = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetFriendCanChangeInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetFriendCanChangeInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetFriendCanChangeInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_num int default 0;
	declare t_synid smallint default 0;
	select count(*) into t_num from t_friend where uid_User=pUid_User;
	if t_num = 0 then
		select 0,0,t_num;
		select 0,'';
	else
		set t_num = 0;
		select count(*) into t_num from t_syndicate,t_syndicatemember,t_friend where t_friend.uid_User = pUid_User and t_syndicatemember.Uid_User=t_friend.uid_Friend and t_syndicate.SynID = t_syndicatemember.SynID;
		select t_actors.uid,t_actors.Level,t_num from t_actors,t_friend where t_friend.uid_User = pUid_User and t_actors.uid = t_friend.uid_Friend;
		if t_num = 0 then
			select 0,'';
		else
			select t_syndicatemember.Uid_User,t_syndicate.SynName  from t_syndicate,t_syndicatemember,t_friend where t_friend.uid_User=pUid_User and t_syndicatemember.Uid_User=t_friend.uid_Friend and t_syndicate.SynID = t_syndicatemember.SynID;
		end if;
	end if;
		
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetFriendDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetFriendDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetFriendDataInfo`(pUid_User bigint unsigned, pUid_Friend bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_uidFriend bigint unsigned default 0;
	declare t_RelationNum int unsigned default 0;
	select uid_User, uid_Friend, RelationNum into t_uidUser, t_uidFriend, t_RelationNum from t_friend where uid_User = pUid_User and uid_Friend = pUid_Friend;
	
	if t_uidUser = 0 then
		select 2;
		select 0,0,0,'',0,0,'',0;
	else
		select 0;
		select uid_Friend,RelationNum,Flag,FriendName,FriendLevel,FriendSex,FriendSynName,bOneCity from t_friend where uid_User = pUid_User and uid_Friend = pUid_Friend;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetFriendEventListDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetFriendEventListDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetFriendEventListDataInfo`(pUid_User bigint unsigned,pTime bigint unsigned)
BEGIN
	declare t_num int default 0;
	delete from t_friend_event where (pTime - Time) >= 86400;
	select count(*) into t_num from t_friend_event where uid_User = pUid_User;
	if t_num = 0 then
		select 0;
		select 0, '', 0;
	else
		select t_num;
		select uid_Friend, Descript, Time from t_friend_event where uid_User = pUid_User;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetFriendListDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetFriendListDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetFriendListDataInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_num	int default 0;
	select count(*) into t_num from t_friend where uid_User = pUid_User limit 0,1;
	select t_num;
	if t_num = 0 then
		select 0, 0,0,'',0,0,'',0,0,0;
	else
		-- 閺囧瓨鏌婃總钘夊几閻ㄥ嫮鐡戠痪褍鎷扮敮顔芥烦
		update t_friend,t_actors set t_friend.FriendLevel = t_actors.Level where t_friend.uid_Friend = t_actors.uid and t_friend.uid_User = pUid_User;
		update t_friend set t_friend.FriendSynName = '' where t_friend.uid_User = pUid_User;
		update t_friend,t_syndicate,t_syndicatemember set t_friend.FriendSynName = t_syndicate.SynName where t_friend.uid_Friend = t_syndicatemember.Uid_User and t_syndicatemember.SynID = t_syndicate.SynID and t_friend.uid_User = pUid_User;
		update t_friend,t_chengjiupart set t_friend.FriendTitleID = t_chengjiupart.ActiveTitleID where t_friend.uid_Friend = t_chengjiupart.UserUID and t_friend.uid_User = pUid_User;
		
		-- 瀵版鍩岀紒鎾寸亯闂�
		select uid_Friend, RelationNum,Flag,FriendName, FriendLevel,FriendSex,FriendSynName, bOneCity,FriendFacade,FriendTitleID,VipLevel from t_friend where uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetFriendMsgListDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetFriendMsgListDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetFriendMsgListDataInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_num int default 0;
	select count(*) into t_num from t_friendmsg where uid_User = pUid_User;
	select t_num;
	if t_num = 0 then
		select 0,'',0,0,'',0,0;
	else
		-- 閺囧瓨鏌婄粵澶岄獓閸滃苯搴滃ú?
		update t_friendmsg,t_actors set t_friendmsg.SrcUserLevel = t_actors.Level where t_friendmsg.uid_SrcUser = t_actors.uid and t_friendmsg.uid_User = pUid_User;
		update t_friendmsg set t_friendmsg.SrcUserSynName = '' where t_friendmsg.uid_User = pUid_User;
		update t_friendmsg,t_syndicate,t_syndicatemember set t_friendmsg.SrcUserSynName = t_syndicate.SynName where t_friendmsg.uid_SrcUser = t_syndicatemember.Uid_User and t_syndicatemember.SynID = t_syndicate.SynID and t_friendmsg.uid_User = pUid_User;
		update t_friendmsg,t_chengjiupart set t_friendmsg.SrcUserTitleID = t_chengjiupart.ActiveTitleID where t_friendmsg.uid_SrcUser = t_chengjiupart.UserUID and t_friendmsg.uid_User = pUid_User;
		-- 瀵版鍩岀紒鎾寸亯闂�
		select uid_SrcUser,SrcUserName,SrcUserLevel,SrcUserSex,SrcUserSynName, bOneCity, SrcUserFacade,SrcUserTitleID from t_friendmsg where uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetFuMoDongInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetFuMoDongInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetFuMoDongInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_fumodong where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select pUid_User,0,0,0,0,0,0;
	else
		select 0;
		select * from t_fumodong where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetSynMemberLevelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetSynMemberLevelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetSynMemberLevelInfo`(pSynID smallint unsigned)
BEGIN
	declare t_num int default 0;
	select count(*) into t_num from t_syndicatemember where t_syndicatemember.SynID=pSynID;
	if t_num = 0 then
		select 0;
		select 0,0;
	else
		select t_num;
		select t_syndicatemember.Uid_User,t_actors.Level from t_syndicatemember,t_actors where t_syndicatemember.SynID = pSynID and t_actors.uid = t_syndicatemember.Uid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetTalismanWorldRecordInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetTalismanWorldRecordInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetTalismanWorldRecordInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_num int unsigned default 0;
	select count(*) into t_num from t_talismanworldRecord where Uid_User = pUid_User;
	if t_num = 0 then
		select pUid_User,0;
		select 0,0,0,0;
	else
		select pUid_User,t_num;
		select * from t_talismanworldRecord where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetTradeDataListInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetTradeDataListInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetTradeDataListInfo`(pValue int)
BEGIN
	declare t_num	int unsigned default 0; 
	
	select count(*) into t_num from t_trade;
	 if t_num = 0 then     
		select 0;
		select 0,0, '', 0, 0, 0, 0, '';
	 else
	 	select t_num;
	 	select Uid_Goods,Uid_Seller, SellerName, Price, GoodsCategory, SubClass, Time, GoodsData from t_trade;
	 end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_GetTrainingHallInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_GetTrainingHallInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_GetTrainingHallInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_traininghall where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select pUid_User,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_traininghall where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ActivityCnfg` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ActivityCnfg` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ActivityCnfg`()
BEGIN
	declare t_activityid smallint unsigned default 0;
	
	select activityid into t_activityid from t_activitycnfg limit 0,1;
	if t_activityid = 0 then
		select 2;
		select 0,'',0,'',0,0,0,0,0,'',0,'',0,0,'','','','';
	else
		select 0;
		select activityid,name,type,rules,UNIX_TIMESTAMP(begintime),UNIX_TIMESTAMP(endtime),resid,bfinish,eventid,param,attainnum,awarddesc,ticket,godstone,vectgoods,mailsubject,mailcontent,resFileUrl from t_activitycnfg;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ActivityData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ActivityData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_ActivityData`(pUidActor bigint unsigned)
BEGIN
         declare t_uidUser bigint unsigned default 0;
         select uidActor into t_uidUser from t_activity where uidActor = pUidActor;
         if t_uidUser = 0 then
              insert into t_activity(uidActor) value (pUidActor);
         end if;
         select SignInNumOfMonth,LastSignInTime,AwardHistory,OnLineAwardID,LastOnLineAwardRestTime,bCanTakeOnlineAward,OffLineNum,NewPlayerGuideContext from t_activity where uidActor = pUidActor;
         set t_uidUser = 0;
         select uidActor into t_uidUser from t_activityData where uidActor = pUidActor limit 0,1;
         
          if t_uidUser = 0 then
              select 0,0,0,0;
          else
              select ActivityID,bFinished,bTakeAward,ActivityProgress  from t_activityData where uidActor = pUidActor ;
         end if;
 END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_CDTimer` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_CDTimer` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_CDTimer`(pUidActor bigint unsigned)
BEGIN
	declare t_timerID smallint unsigned default 0;
	select CDTimerID into t_timerID from t_cdtimer where UidActor = pUidActor limit 0,1;
	select pUidActor;
	if t_timerID = 0 then
		select 0, 0;
	else
		select CDTimerID, EndTime from t_cdtimer where UidActor = pUidActor;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ChallengeLevelRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ChallengeLevelRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ChallengeLevelRank`(pBeginLevel tinyint unsigned, pEndLevel tinyint unsigned, pBeginPos int, pNum int)
BEGIN
	declare t_rank int unsigned default 0;
	select Rank into t_rank from t_challengeRank limit 0,1;
	if t_rank = 0 then
		select 2;
		select 0,0,0,0,0,0,0;
	else
		set pBeginPos = pBeginPos - 1;
		select 0;
		select * from t_challengeRank where level >= pBeginLevel and level <= pEndLevel order by Rank asc limit pBeginPos, pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ChallengeMyEnemy` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ChallengeMyEnemy` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ChallengeMyEnemy`(puidUser bigint unsigned,pNum int)
BEGIN
	declare t_beginrank int default 0;
	declare t_myrank int default 0;
	declare t_level tinyint unsigned default 0;
	declare t_myranklevel int default 0;
	select Rank,level into t_myrank,t_level from t_challengerank where uidUser=puidUser;
	select count(*) into t_myranklevel from t_challengerank where Rank < t_myrank and ((level - level % 10) / 10 = (t_level - t_level % 10) / 10);
	set t_myranklevel = t_myranklevel + 1;
	
	if t_myrank = 0 then
		select 2,0,0;
		select 0,0,0,0,0,0,0,0;
	else
		set t_beginrank = t_myrank - pNum - 1;
		if t_beginrank < 0 then
			set t_beginrank = 0;
			set pNum = t_myrank - 1;
		end if;
		select 0,t_myrank,t_myranklevel;
		select * from t_challengerank order by Rank asc limit t_beginrank, pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ChallenteRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ChallenteRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ChallenteRank`(pBeginRank int unsigned, pNum int unsigned)
BEGIN
	declare t_rank int unsigned default 0;
	declare t_maxrank int unsigned default 0;
	select Rank into t_rank from t_challengerank where Rank = 1;
	if t_rank = 0 then
		select 2;
		select 0,0,'',0,0,0,0;
	else
		select 0;
		set pBeginRank = pBeginRank - 1;
		select max(Rank) into t_maxrank from t_challengerank;
		if pBeginRank + pNum > t_maxrank + 1 then
			set pNum = t_maxrank - pBeginRank + 1;
		end if;
		
		select * from t_challengerank order by Rank asc limit pBeginRank,pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ChengJiuData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ChengJiuData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ChengJiuData`(pUidUser bigint unsigned)
BEGIN
	declare t_uid bigint unsigned default 0;
	declare t_ChengJiuID smallint unsigned default 0;
	select UserUID into t_uid from t_chengjiupart where UserUID = pUidUser;
	select ChengJiuID into t_ChengJiuID from t_chengjiu where UserUID = pUidUser limit 0, 1;
	if t_uid = 0 then
		select pUidUser,0,0,0;
	else
		select * from t_chengjiupart where UserUID = pUidUser;
	end if;
	if t_ChengJiuID = 0 then
		select 0,0,0;
	else
		select ChengJiuID,CurCount,FinishTime from t_chengjiu where UserUID = pUidUser;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_CombatInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_CombatInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_CombatInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select  Uid_User into t_uidUser from t_combat where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 0;
		select pUid_User,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_combat where Uid_User=pUid_User;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_CreditRand` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_CreditRand` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_CreditRand`()
BEGIN
	select 0;
	select uid,Name,Credit,Level from t_actors where uidMaster = 0 order by Credit desc, Level desc;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_EquipPanelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_EquipPanelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_EquipPanelInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_equipNum tinyint unsigned default 0;
	select count(*) into t_equipNum from t_goods where Uid_User = pUid_User and PositionGoods = 1;
	select pUid_User,t_equipNum;
	if t_equipNum = 0 then
		select 0,0,0,0,0,0,'';
	else
		select location,UidGoods,IDGoods,CreateTime,Number,Binded,BinData from t_goods where Uid_User = pUid_User and PositionGoods = 1;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_FuBenInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_FuBenInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_FuBenInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_fubenID tinyint unsigned default 0;
	select Uid_User into t_uidUser from t_fubenbasic where Uid_User=pUid_User;
 
	if t_uidUser = 0 then
		select pUid_User,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
		select 0,0,0,0;
	else
		select FuBenID into t_fubenID from t_fubenprogress where Uid_User=pUid_User limit 0,1;
		if t_fubenID = 0 then
			select * from t_fubenbasic where Uid_User=pUid_User;
			select 0,0,0,0;
		else
			select * from t_fubenbasic where Uid_User=pUid_User;
			select * from t_fubenprogress where Uid_User = pUid_User;
		end if;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_GatherGodHouseInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_GatherGodHouseInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_GatherGodHouseInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_gathergodhouse where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select pUid_User,0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_gathergodhouse where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_GodSwordShopInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_GodSwordShopInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_GodSwordShopInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_godswordshop where Uid_User = pUid_User;
	if t_uidUser = 0 then
		select 2;
		select pUid_User,0,0,0,0,0;
	else
		select 0;
		select * from t_godswordshop where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_MagicPanelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_MagicPanelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_MagicPanelInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_numMagic tinyint unsigned default 0;
	select count(*) into t_numMagic from t_magic where Uid_User = pUid_User;
	select pUid_User,t_numMagic;
	if t_numMagic = 0 then
		select 0,0,0;
	else
		select MagicID,level,MagicPosition from t_magic where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_MailDataList_Req` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_MailDataList_Req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_MailDataList_Req`(pUid_User bigint unsigned)
BEGIN
	declare t_NumMail int unsigned default 0;
	select count(*) into t_NumMail from t_mail where uid_User = pUid_User;
	select t_NumMail;
	if t_NumMail = 0 then
		select 0,0,'',0,0,0,0,0,'','',0,'';
	else
		select MailID, uid_SendUser, Name_SendUser, Mail_Type,stone, money,ticket,SynID,ThemeText, ContentText,Time, GoodsData from t_mail where uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_MailNum` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_MailNum` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_MailNum`(pUidUser bigint unsigned)
BEGIN
	declare t_MailNum smallint unsigned default 0;
	
	select count(*) into t_MailNum from t_mail where uid_User = pUidUser;
	select t_MailNum;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_MyChallengeRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_MyChallengeRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_MyChallengeRank`(pUidUser bigint unsigned, pNum smallint unsigned)
BEGIN
	declare t_rank int default 0;
	declare t_beginrank int default 0;
	declare t_maxrank int default 0;
	select Rank into t_rank from t_challengerank where uidUser = pUidUser;
	if t_rank = 0 then
		select 2;
		select 0,0,'',0,0,0,0;
	else
		set t_beginrank = t_rank - pNum / 2;
		if t_rank <= pNum then
			set t_beginrank = 1;
		end if;
		
		select max(Rank) into t_maxrank from t_challengerank;
		if t_rank + pNum > t_maxrank then
			-- set t_beginrank = t_maxrank - pNum;
			set pNum = t_maxrank - t_beginrank;
		end if;
		
		if t_beginrank < 0 then
			set t_beginrank = 1;
		end if;
		set t_beginrank = t_beginrank - 1;
		select 0;
		select * from t_challengerank order by Rank asc limit t_beginrank, pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_MyRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_MyRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_MyRank`(pUidUser bigint unsigned, pNum smallint unsigned)
BEGIN
	declare t_rank int default 0;
	declare t_beginrank int default 0;
	declare t_maxrank int default 0;
	select Rank into t_rank from t_userrank where uidUser = pUidUser;
	if t_rank = 0 then
		select 2;
		select 0,0,'',0,0,0;
	else
		set t_beginrank = t_rank - pNum / 2;
		if t_rank <= pNum then
			set t_beginrank = 1;
		end if;
		
		select max(Rank) into t_maxrank from t_userrank;
		if t_rank + pNum > t_maxrank then
			-- set t_beginrank = t_maxrank - pNum;
			set pNum = t_maxrank - t_beginrank;
		end if;
		
		if t_beginrank < 0 then
			set t_beginrank = 1;
		end if;
		set t_beginrank = t_beginrank - 1;
		select 0;
		select * from t_userrank order by Rank asc limit t_beginrank, pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_OneGoodsShopCnfg` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_OneGoodsShopCnfg` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_OneGoodsShopCnfg`(pID int unsigned)
BEGIN
	declare t_id int unsigned default 0;
	select id into t_id from t_shopmall where id = pID;
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select id,goodsid,num,type,moneytype,moneynum,binded,UNIX_TIMESTAMP(starttime),UNIX_TIMESTAMP(endtime),leftFen from t_shopmall where id = pID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_PacketInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_PacketInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_PacketInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uid_user bigint unsigned default 0;
	declare t_goodID smallint unsigned default 0;
	select Uid_User into t_uid_user from t_packet where Uid_User=pUid_User;
	if t_uid_user = 0 then
		select pUid_User,0;
		select 0,0,0,0,0,0,'';
	else
		select IDGoods into t_goodID from t_goods where Uid_User = pUid_User and PositionGoods = 0 limit 0,1;
		select Uid_User,Capacity from t_packet where Uid_User=pUid_User;
		
		if t_goodID = 0 then	
			select 0,0,0,0,0,0,'';
		else
			select location,UidGoods,IDGoods,CreateTime,Number,Binded,BinData from t_goods where Uid_User = pUid_User and PositionGoods = 0;
		end if;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_RankSection` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_RankSection` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_RankSection`(pBeginRank int unsigned, pNum smallint unsigned)
BEGIN
	declare t_rank int unsigned default 0;
	declare t_maxrank int unsigned default 0;
	select Rank into t_rank from t_userrank where Rank = 1;
	if t_rank = 0 then
		select 2;
		select 0,0,'',0,0,0;
	else
		select 0;
		set pBeginRank = pBeginRank - 1;
		select max(Rank) into t_maxrank from t_userrank;
		if pBeginRank + pNum > t_maxrank + 1 then
			set pNum = t_maxrank - pBeginRank + 1;
		end if;
		
		select * from t_userrank order by Rank asc limit pBeginRank,pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_SectionMailData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_SectionMailData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_SectionMailData`(puid_User bigint unsigned, pBegin smallint, pNum smallint)
BEGIN
	declare t_mailID int unsigned default 0;
	select MailID into t_mailID from t_mail where uid_User = puid_User limit 0, 1;
	if t_mailID = 0 then
		select 2;
		select 0,0,'',0,0,0,0,0,'','',0,0,0;
	else
		select 0;
		select  MailID, uid_SendUser, Name_SendUser, Mail_Type,stone, money,ticket,SynID,ThemeText, ContentText,Time, bRead,GoodsData from t_mail where uid_User = puid_User order by MailID desc limit pBegin,pNum;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_ShopMallCnfg` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_ShopMallCnfg` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_ShopMallCnfg`()
BEGIN
	declare t_id smallint unsigned default 0;
	delete from t_shopmall where (now() > endtime and endtime <> 0) or leftFen = 0;
 
	select id into t_id from t_shopmall limit 0,1;
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select id,goodsid,num,type,moneytype,moneynum,binded,UNIX_TIMESTAMP(starttime),UNIX_TIMESTAMP(endtime),leftFen from t_shopmall;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_StatusInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_StatusInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_StatusInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_statusID smallint unsigned default 0;
	select StatusID into t_statusID from t_status where Uid_User = pUid_User limit 0,1;
	if t_statusID = 0 then
		select pUid_User, 0;
		select 0,0,0;
	else
		select pUid_User, 0;
		select StatusID, EndStatusTime, UidCreator from t_status where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_SyndicateApplyListInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_SyndicateApplyListInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_SyndicateApplyListInfo`()
BEGIN
	declare t_num	int default 0; 
	select count(*) into t_num from t_syndicateapplyer;
	if t_num = 0 then
		select 0;
		select 0,0,'',0,0;
	else
		select t_num;
		update t_syndicateapplyer,t_actors set t_syndicateapplyer.ApplyUserLevel = t_actors.Level,t_syndicateapplyer.ApplyUserLayer = t_actors.ActorLayer where t_syndicateapplyer.Uid_ApplyUser = t_actors.uid;
		select * from t_syndicateapplyer;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_SyndicateListInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_SyndicateListInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_SyndicateListInfo`()
BEGIN
	declare t_synnum	int default 0; 
	select count(*) into t_synnum from t_syndicate;
	if t_synnum = 0 then
		select 0;
		select 0,'',0,'',0,0,0;
	else
		select t_synnum;
		select * from t_syndicate;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_SyndicateMemberListInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_SyndicateMemberListInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_SyndicateMemberListInfo`()
BEGIN
	declare t_num	int default 0; 
	select count(*) into t_num from t_syndicatemember;
	if t_num = 0 then
		select 0;
		select 0,0,'',0,0,0,0;
	else
		select t_num;
		update t_syndicatemember,t_actors set t_syndicatemember.Userlevel = t_actors.Level where t_syndicatemember.Uid_User = t_actors.uid;
		select * from t_syndicatemember;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_SynMagicInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_SynMagicInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_SynMagicInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_numMagic tinyint unsigned default 0;
	select count(*) into t_numMagic from t_syndicatemagic where Uid_User = pUid_User;
	select pUid_User,t_numMagic;
	if t_numMagic = 0 then
		select 0,0;
	else
		select SynMagicID ,Level from t_syndicatemagic where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_TaskInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_TaskInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_TaskInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_UidUser bigint default 0;
        	
	select Uid_User into t_UidUser from t_taskpart where Uid_User=pUid_User;
	if t_UidUser = 0 then
		select  0, 0, 0,0,0;
	else
		select TaskHistory,LastUpdateTime,NotSaveNum, TaskStatus, NotSaveTaskID from t_taskpart where Uid_User = pUid_User;
	end if;
         set t_UidUser = 0;
	 select Uid_User into t_UidUser from t_task where Uid_User = pUid_User limit 0,1;
	if t_UidUser = 0 then 
		select 0,0,0;
	else
		select TaskID, FinishTime, CurCount from t_task where Uid_User = pUid_User;
	end if;
		
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_UserCDTimer` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_UserCDTimer` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_UserCDTimer`(pUidActor bigint unsigned)
BEGIN
	declare t_timerID smallint unsigned default 0;
	select CDTimerID into t_timerID from t_cdtimer where UidActor = pUidActor limit 0,1;
	select pUidActor;
	if t_timerID = 0 then
		select 0, 0;
	else
		select CDTimerID, EndTime from t_cdtimer where UidActor = pUidActor;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_UserRand` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_UserRand` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_UserRand`()
BEGIN
	select 0;
	select uid, Level,ActorExp,Credit, ActorLayer,Name from t_actors where uidMaster = 0 order by Level desc,ActorExp desc; 
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_XiuLianInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_XiuLianInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_XiuLianInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_xiulian where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select pUid_User,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_xiulian where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_XiuLianInfo2` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_XiuLianInfo2` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Get_XiuLianInfo2`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_xiulian where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select pUid_User,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0;
		select 0,0,0,0,0,0,0,0,0,0,'',0,0,'',0,0,0,0,0,0,0,0,0,0,0;
	else
		select * from t_xiulian where Uid_User = pUid_User;
		select * from t_xiulian_record where uidsource = pUid_User or uidFriend = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertActorInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertActorInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertActorInfo`(pUserID int unsigned,pName varchar(18),pActorData blob(65535))
BEGIN
      declare t_userid int unsigned default 0;
      declare t_actordata blob(65535);
      declare t_count int default 0;
      select UserID, ActorData into t_userid,t_actordata from t_actor where UserID = pUserID;
      if t_userid = 0 then
          select count(*) into t_count from t_actor where ActorName = pName;
          if t_count = 0 then
             insert into t_actor(UserID, ActorName,ActorData ) values(pUserID,pName,pActorData);
             select UserID, ActorData into t_userid,t_actordata from t_actor where UserID = pUserID;        
          end if;
        
      end if;
       if t_userid = 0 then
            select 4; 
          else
            select 0;  
       end if;
      select t_userid,t_actordata;
      
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertBuildingInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertBuildingInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertBuildingInfo`(pBuildingType tinyint unsigned, pUid bigint unsigned, pGuardUid bigint unsigned,pGuardOthter bigint unsigned,pUidOwner bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_buildingpart where Uid_User = pUidOwner and BuildingType = pBuildingType;
	if t_uidUser = 0 then
		insert into t_building(Uid_Building,BuildingType,GuardUid,Uid_Owner) values(pUid,pBuildingType,pGuardUid,pUidOwner);
		insert into t_buildingpart(Uid_User,BuildingType,uidBuilding,GuardOthter) values(pUidOwner,pBuildingType,pUid,pGuardOthter);
	end if;
	select Uid_User into t_uidUser from t_buildingpart where Uid_User = pUidOwner and BuildingType = pBuildingType;
	if t_uidUser = 0 then
		select pUidOwner,0,0,0,0,0;
		select 0,0,0,0,0,0,'','',0;
	else
		select * from t_buildingpart where Uid_User = pUidOwner and BuildingType = pBuildingType;
		select * from t_building where Uid_Owner=pUidOwner or GuardUid=pUidOwner;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertDouFaInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertDouFaInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_InsertDouFaInfo`(pUserUid bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select UserUid into t_uidUser from t_doufa where UserUid = pUserUid;
	
	if t_uidUser = 0 then
		insert into t_doufa(UserUid) values(pUserUid);
		select UserUid into t_uidUser from t_doufa where UserUid = pUserUid;
	end if;
	
	if t_uidUser = 0 then
		select 2;
		select pUserUid, 0, 0,0,0,0;
	else
		select 0;
		select * from t_doufa where UserUid = pUserUid;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertFriendDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertFriendDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertFriendDataInfo`(pUid_User bigint unsigned, pUid_Friend bigint unsigned, pRelation int unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_num int unsigned default 0;
	declare t_relationNum int unsigned default 0;
	declare t_flag tinyint unsigned default 0;
	declare t_friendName varchar(18) default '';
	declare t_friendLevel tinyint unsigned default 0;
	declare t_friendSex   tinyint unsigned default 0;
	declare t_friendSynName varchar(18) default '';
	declare t_friendFacaed smallint unsigned default 0;
	declare t_cityID smallint unsigned default 0;
	declare t_friendCityID smallint unsigned default 0;
	declare t_bOneCity bool default false;
	declare t_MeName varchar(18) default '';
	declare t_MeLevel tinyint unsigned default 0;
	declare t_MeSex	tinyint unsigned default 0;
	declare t_MeSynName varchar(18) default '';
	declare t_Facaed smallint unsigned default 0;
	declare t_synid smallint default 0;
	declare t_friendtitleID tinyint unsigned default 0;
	declare t_titleID tinyint unsigned default 0;
	select Name,Level,ActorSex,CityID,ActorFacade into t_MeName,t_MeLevel,t_MeSex,t_cityID,t_Facaed from t_actors where uid=pUid_User;
	select Name,Level,ActorSex,CityID,ActorFacade into t_friendName,t_friendLevel,t_friendSex,t_friendCityID,t_friendFacaed from t_actors where uid=pUid_Friend;
	select ActiveTitleID into t_friendtitleID from t_chengjiupart where t_chengjiupart.UserUID = pUid_Friend;
	select ActiveTitleID into t_titleID from t_chengjiupart where t_chengjiupart.UserUID = pUid_User;
	if t_cityID = t_friendCityID then
		 set t_bOneCity = true;
	end if;
	select uid_User into t_uidUser from t_friend where uid_User = pUid_User and uid_Friend = pUid_Friend;
	if t_uidUser = 0 then
		-- 閺勵垰宕熼崥鎴濄偨閸欏绻曢弰顖氬蓟閸氭垵銈介崣?0娑撳搫宕熼崥?1娑撳搫寮婚崥?
		select uid_User,RelationNum into t_uidUser,t_relationNum from t_friend where uid_User = pUid_Friend and uid_Friend = pUid_User;
		if t_uidUser = 0 then
			set t_flag = 0;
			-- 瀵扳偓鐎佃鏌熼崝鐘虫蒋婵傝棄寮告穱鈩冧紖
			select count(*) into t_num from t_friendmsg where uid_User = pUid_Friend;
			if t_num < 100 then
				select SynID into t_synid from t_syndicatemember where Uid_User=pUid_Friend;
				if t_synid=0 then
					set t_MeSynName = '';
				else
					select 	SynName into t_MeSynName from t_syndicate where SynID=t_synid;
				end if;
				insert into t_friendmsg(uid_User, uid_SrcUser,SrcUserName,SrcUserLevel,SrcUserSex,SrcUserSynName,bOneCity,SrcUserFacade,SrcUserTitleID) values(pUid_Friend, pUid_User,t_MeName,t_MeLevel,t_MeSex,t_MeSynName,t_bOneCity,t_Facaed,t_titleID);
			end if;
		else
			set t_flag = 1;
			-- 閸ョ姳璐熸總钘夊几鎼达附妲搁崣灞炬煙閻ㄥ嫬鍙￠崥灞界潣閹�閹碘偓娴犮儵鈧瀚ㄦ径褏娈�
			if t_relationNum > pRelation then
				set pRelation = t_relationNum;
			end if;
			update t_friend set Flag = t_flag,RelationNum = pRelation where uid_User = pUid_Friend and uid_Friend = pUid_User;
			-- 閹跺﹤銈介崣瀣╀繆閹垰鍨归梽?
			delete from t_friendmsg where uid_User = pUid_User and uid_SrcUser = pUid_Friend;
		end if;
		set t_synid = 0;
		select SynID into t_synid from t_syndicatemember where Uid_User=pUid_Friend;
		if t_synid=0 then
			set t_friendSynName = '';
		else 
			select 	SynName into t_friendSynName from t_syndicate where SynID=t_synid;
		end if;
		
		insert into t_friend(uid_User, uid_Friend, RelationNum,Flag,FriendName,FriendLevel,FriendSex,FriendSynName,bOneCity,FriendFacade,FriendTitleID) values(pUid_User, pUid_Friend, pRelation,t_flag,t_friendName,t_friendLevel,t_friendSex,t_friendSynName,t_bOneCity,t_friendFacaed,t_friendtitleID);
		select 0;
		select pUid_Friend,pRelation,t_friendName,t_friendLevel,t_friendSex,t_friendSynName,t_flag,t_bOneCity,t_friendFacaed,t_friendtitleID;
	else
		select 2;
		select 0,0,'',0,0,'',0,0,0,0;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertFriendEnventDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertFriendEnventDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertFriendEnventDataInfo`(pUid_User bigint unsigned, pUid_Friend bigint unsigned, pAddRelationNum int unsigned, pDescript varchar(100), pTime bigint unsigned)
BEGIN
	insert into t_friend_event(uid_User, uid_Friend, AddRelationNum, Descript, Time) value(pUid_User, pUid_Friend, pAddRelationNum, pDescript, pTime);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertFriendMsgDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertFriendMsgDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertFriendMsgDataInfo`(pUid_User bigint unsigned, pUid_SrcUser bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select uid_User into t_uidUser from t_friendmsg where uid_User = pUid_User and uid_SrcUser = pUid_SrcUser;
	if t_uidUser = 0 then
		insert into t_friendmsg(uid_User, uid_SrcUser) values(pUid_User, pUid_SrcUser);
		select 0;
	else
		select 2;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertFuMoDongInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertFuMoDongInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertFuMoDongInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_fumodong where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_fumodong(Uid_User) values(pUid_User);
	end if;
	select Uid_User into t_uidUser from t_fumodong where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select 0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_fumodong where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertSyndicateApplyInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertSyndicateApplyInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertSyndicateApplyInfo`(pSynID smallint unsigned,pUid_ApplyUser bigint unsigned,pApplyUserName varchar(18),pApplyUserLevel tinyint unsigned,pApplyUserLayer tinyint unsigned)
BEGIN
	declare t_synid smallint unsigned default 0;
	select SynID into t_synid from t_syndicateapplyer where SynID = pSynID and Uid_ApplyUser=pUid_ApplyUser;
	if t_synid = 0 then
		insert into t_syndicateapplyer values(pSynID,pUid_ApplyUser,pApplyUserName,pApplyUserLevel,pApplyUserLayer);
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertSyndicateInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertSyndicateInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertSyndicateInfo`(pSynName varchar(18),puid_Leader bigint unsigned,pLeaderName varchar(18))
BEGIN
	declare t_synid smallint unsigned default 0;
	insert into t_syndicate(SynName,Uid_Leader,LeaderName) values(pSynName,puid_Leader,pLeaderName);
	select SynID into t_synid from t_syndicate where SynName = pSynName;
	if t_synid = 0 then
		select 2;
		select 0;
	else
		select 0;
		select t_synid;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertSyndicateMemberInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertSyndicateMemberInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertSyndicateMemberInfo`(puid_User bigint unsigned,pSynID smallint unsigned,pUserName varchar(18),pUserLevel tinyint unsigned,pPosition tinyint unsigned,pContribution int unsigned,pVipLevel tinyint unsigned)
BEGIN
	declare t_uiduser bigint unsigned default 0;
	select Uid_User into t_uiduser from t_syndicatemember where Uid_User=puid_User;
	if t_uiduser = 0 then
		insert into t_syndicatemember(Uid_User,SynID,UserName,Userlevel,Position,Contribution,VipLevel) values(puid_User,pSynID,pUserName,pUserLevel,pPosition,pContribution,pVipLevel);
		delete from t_syndicateapplyer where Uid_ApplyUser = puid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertTrainingHallInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertTrainingHallInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_InsertTrainingHallInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_traininghall where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_traininghall(Uid_User) values(pUid_User); 
	end if;
	select Uid_User into t_uidUser from t_traininghall where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select 0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_traininghall where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_InsertUserRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_InsertUserRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_InsertUserRank`(pUidUser bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_Name varchar(18) default "";
	declare t_level tinyint unsigned default 0;
	declare t_exp	int unsigned default 0;
	declare t_layer int unsigned default 0;
	declare t_maxrank int unsigned default 0;
	select uid,Name,Level,ActorExp,ActorLayer into t_uidUser, t_Name, t_level, t_exp, t_layer from t_actors where uid = pUidUser;
	select max(Rank) into t_maxrank from t_userrank;
	set t_maxrank = t_maxrank + 1;
	insert into t_userrank(Rank,uidUser,Name,Level,Exp,Layer) values(t_maxrank,t_uidUser,t_Name, t_level,t_exp,t_layer); 
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_ActorBasicInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_ActorBasicInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_ActorBasicInfo`(pUserID int unsigned,pName varchar(18),pLevel tinyint unsigned,pSpirit int unsigned,pShield int unsigned,pBlood int unsigned,pAvoid int unsigned,pActorExp int unsigned,pActorLayer int unsigned,pActorNimbus int unsigned,
										pActorAptitude int unsigned,pActorSex tinyint unsigned,pUid bigint unsigned,pActorFacade smallint unsigned,pUidMaster bigint unsigned,pCityID smallint unsigned,pActorNimbusSpeed smallint unsigned)
BEGIN
	declare t_userid int unsigned default 0;
	declare t_count int default 0;
	select UserID into t_userid from t_actors where UserID = pUserID;
	if t_userid = 0 then
		select count(*) into t_count from t_actors where Name = pName limit 0,1;
		if t_count = 0 then
			insert into t_actors(UserID,Name,Level,Spirit,Shield,Avoid,ActorExp,ActorLayer,ActorNimbus,ActorAptitude,ActorSex,uid,ActorFacade,actorbloodup,uidMaster,CityID,ActorNimbusSpeed,UserFlag) 
                                     values(pUserID,pName,pLevel,pSpirit,pShield,pAvoid,pActorExp,pActorLayer,pActorNimbus,pActorAptitude,pActorSex,pUid,pActorFacade,pBlood,pUidMaster,pCityID,pActorNimbusSpeed,'\0');
			select UserID into t_userid from t_actors where UserID = pUserID;
		end if;
	end if;
	if t_userid = 0 then
		select 4;
		select 0,'',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_actors where UserID = pUserID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_bigint_test` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_bigint_test` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_bigint_test`(pBigint bigint unsigned)
BEGIN
	insert into bigint_test values(pBigint);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_ChallengeRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_ChallengeRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_ChallengeRank`(pUidUser bigint unsigned, pName varchar(18))
BEGIN
	declare t_rank int unsigned default 0;
	declare t_maxrank int unsigned default 0;
	select Rank into t_rank from t_challengerank where uidUser = pUidUser;
	
	if t_rank = 0 then
		select max(Rank) into t_maxrank from t_challengerank;
		set t_maxrank = t_maxrank + 1;
		insert into t_challengerank(Rank,uidUser,Name) values(t_maxrank,pUidUser, pName);
		select 0;
		select t_maxrank;
	else
		select 0;
		select t_rank;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_ChengJiuPart` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_ChengJiuPart` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_ChengJiuPart`(pUidUser bigint unsigned)
BEGIN
	declare t_uid bigint unsigned default 0;
	insert into t_chengjiupart(UserUID) values(pUidUser);
	select UserUID into t_uid from t_chengjiupart where UserUID = pUidUser;
	if t_uid = 0 then
		select pUidUser,0,0,0;
	else
		select * from t_chengjiupart where UserUID = pUidUser;
	end if;
	select 0,0,0;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_insert_CombatInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_insert_CombatInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_insert_CombatInfo`(pUid_User bigint unsigned,puidLineup1 bigint unsigned,puidLineup2 bigint unsigned,puidLineup3 bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_uidLineup1 bigint unsigned default 0;
	declare t_uidLineUp2 bigint unsigned default 0;
	declare t_uidLineUp3 bigint unsigned default 0;
	select  Uid_User,uidLineup1,uidLineup2,uidLineup3 into t_uidUser,t_uidLineup1,t_uidLineUp2,t_uidLineUp3 from t_combat where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_combat values(pUid_User,puidLineup1,puidLineup2,puidLineup3);
		select  Uid_User,uidLineup1,uidLineup2,uidLineup3 into t_uidUser,t_uidLineup1,t_uidLineUp2,t_uidLineUp3 from t_combat where Uid_User=pUid_User;
	end if;
	if t_uidUser = 0 then
		select 2;
		select 0,0,0,0;
	else
		select 0;
		select t_uidUser,t_uidLineup1,t_uidLineUp2,t_uidLineUp3;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_EquipPanelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_EquipPanelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_EquipPanelInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_numEquip tinyint unsigned default 0;
	select count(*) into t_numEquip from t_goods where Uid_User = pUid_User and PositionGoods = 1; 
	select pUid_User,t_numEquip;
	if t_numEquip = 0 then
		select 0,0,0,0,0,0,'';
	else
		select location,UidGoods,IDGoods,CreateTime,Number,Binded,BinData from t_goods where Uid_User = pUid_User and PositionGoods = 1; 
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_FuBenInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_FuBenInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_FuBenInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_fubenID tinyint unsigned default 0;
	select Uid_User into t_uidUser from t_fubenbasic where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_fubenbasic(Uid_User) values(pUid_User);
		select Uid_User into t_uidUser from t_fubenbasic where Uid_User=pUid_User;
	end if;
	if t_uidUser = 0 then
		select pUid_User,0,0,0,0,0,0,0,0;
		select 0,0,0,0;
	else
		select FuBenID into t_fubenID from t_fubenprogress where Uid_User=pUid_User limit 0,1;
		if t_fubenID = 0 then
			select t_uidUser,t_lastFreeEnterFuBenTime,t_lastCostEnterFuBenTime,t_freeEnterFuBenNum,t_SynWelfareEnterFuBenNum,t_CostStoneEnterFuBenNum,t_costEnterFuBenNum,t_fuBenNum;
			select 0,0,0,0;
		else
			select t_uidUser,t_lastFreeEnterFuBenTime,t_lastCostEnterFuBenTime,t_freeEnterFuBenNum,t_SynWelfareEnterFuBenNum,t_CostStoneEnterFuBenNum,t_costEnterFuBenNum,t_fuBenNum;
			select * from t_fubenprogress where Uid_User = pUid_User;
		end if;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_GatherGodHouseInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_GatherGodHouseInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_GatherGodHouseInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_gathergodhouse where Uid_User=pUid_User;
	if t_uidUser=0 then
		insert into t_gathergodhouse(Uid_User) values(pUid_User);
	end if;
	select Uid_User into t_uidUser from t_gathergodhouse where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 2;
		select 0,0,0,0,0,0,0,0,0,0,0;
	else
		select 0;
		select * from t_gathergodhouse where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_GodSwordShopInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_GodSwordShopInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_GodSwordShopInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_godswordshop where Uid_User = pUid_User;
	if t_uidUser = 0 then
		insert into t_godswordshop(Uid_User) values(pUid_User);
		select Uid_User into t_uidUser from t_godswordshop where Uid_User=pUid_User;
	end if;
	if t_uidUser = 0 then
		select 4;
		select 0,0,0,0,0,0;
	else
		select 0;
		select * from t_godswordshop where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_MagicPanelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_MagicPanelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_MagicPanelInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_numMagic tinyint unsigned default 0;
	select count(*) into t_numMagic from t_magic where Uid_User = pUid_User;
	select pUid_User,t_numMagic;
	if t_numMagic = 0 then
		select 0,0,0;
	else
		select MagicID,level,MagicPosition from t_magic where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_MailData_Req` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_MailData_Req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_MailData_Req`(pUid_User bigint unsigned, pUid_SendUser bigint unsigned, pSendUserName varchar(18),  pMail_Type tinyint unsigned,pStone int unsigned, pMoney int unsigned,
                   pTicket int unsigned, pSynID smallint unsigned, pThemeText varchar(50), 
                    pContentText varchar(300), pTime bigint unsigned , pGoodsData varbinary(1024))
BEGIN
	declare t_MailID int unsigned default 0;
	declare t_NumRecord int unsigned default 0;
	declare t_Time bigint	unsigned default 0;
	
        select  MailID into t_MailID from t_mail where uid_User = pUid_User order by MailID desc limit 300,1;
        if t_MailID <> 0 then
           delete from t_mail where MailID = t_MailID;
         end if;
       	
	insert into t_mail(uid_User, uid_SendUser, Name_SendUser, Mail_Type,stone, money, ticket, SynID, ThemeText, ContentText, Time, GoodsData) 
          values(pUid_User, pUid_SendUser, pSendUserName, pMail_Type,pStone, pMoney, pTicket, pSynID, pThemeText, pContentText, pTime, pGoodsData);
	
	set t_MailID = LAST_INSERT_ID();
	if t_MailID = 0 then
		select 3;
		select t_MailID,pUid_User;
	else
		select 0;
		select t_MailID,pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_PacketInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_PacketInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_PacketInfo`(pUid_User bigint unsigned,pCapacity smallint  unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_numGoods int unsigned default 0;
	select Uid_User into  t_uidUser from t_packet where Uid_User = pUid_User;
	if t_uidUser = 0 then
		insert into t_packet(Uid_User,Capacity) values(pUid_User,pCapacity);
	end if;
	
	select Uid_User into t_uidUser from t_packet where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select 0,0;
	else	
		select Uid_User,Capacity from t_packet where Uid_User=pUid_User;
	end if;
	select count(*) into t_numGoods from t_goods where Uid_User = pUid_User and PositionGoods = 0;
	if t_numGoods = 0 then
		select 0,0,0,0,0,0,'';
	else
		select location,UidGoods,IDGoods,CreateTime,Number,Binded,BinData from t_goods where Uid_User = pUid_User and PositionGoods = 0;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_ResBuildRecord` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_ResBuildRecord` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_ResBuildRecord`(pUidUser bigint unsigned, pBuildingType tinyint unsigned, pHappenTime int unsigned, pszTargetName varchar(18), pszHandleName varchar(18), pszContext varchar(50))
BEGIN
	insert into t_buildrecord values(pUidUser,pBuildingType,pHappenTime,pszTargetName,pszHandleName,pszContext);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_ShopMallLog` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_ShopMallLog` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_ShopMallLog`(pUserID int unsigned, pShopMallID int unsigned,pBuyNum int unsigned)
BEGIN
	insert into t_shopmalllog(userid,shopid,buytime,buynums) values(pUserID,pShopMallID,now(),pBuyNum);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_SynMagicInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_SynMagicInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_SynMagicInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_numMagic tinyint unsigned default 0;
	select count(*) into t_numMagic from t_syndicatemagic where Uid_User = pUid_User;
	select pUid_User,t_numMagic;
	if t_numMagic = 0 then
		select 0,0;
	else
		select SynMagicID ,Level from t_syndicatemagic where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_SysMailByUserID` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_SysMailByUserID` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_SysMailByUserID`(pUserID int unsigned, puidSendUser bigint unsigned,pNameSendUser varchar(18), pMailType tinyint unsigned,
								pStone int unsigned,pMoney int unsigned,pTicket int unsigned,pThemeText varchar(50), pContentText varchar(300),
								pTime bigint unsigned,pGoodsData varbinary(1024))
BEGIN
	declare t_uid bigint unsigned default 0;
	select uid into t_uid from t_actors where UserID = pUserID;
	
	if t_uid <> 0 then
		insert into t_mail(uid_User,uid_SendUser,Name_SendUser,Mail_Type,stone,money,ticket,SynID,ThemeText,ContentText,Time,GoodsData)
		values(t_uid, puidSendUser,pNameSendUser,pMailType,pStone,pMoney,pTicket,0,pThemeText,pContentText,pTime,pGoodsData);
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_TaskInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_TaskInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Insert_TaskInfo`(pUid_User bigint unsigned)
BEGIN
	declare t_useruid bigint unsigned default 0;
	select Uid_User into t_useruid from t_taskpart where Uid_User = pUid_User;
	if t_useruid = 0 then
		insert into t_taskpart values(pUid_User, 0, 0, 0, 0);
	end if;
	set t_useruid = 0;
	select Uid_User into t_useruid from t_taskpart where Uid_User=pUid_User;
	if t_useruid = 0 then 
		select 0, 0, 0, 0;
	else
		select TaskHistory, LastUpdateTime, NotSaveNum, NotSaveTaskID from t_taskpart where Uid_User = pUid_User;
	end if;
	set t_useruid = 0;
	select Uid_User into t_useruid from t_task where Uid_User = pUid_User limit 0,1;
	if t_useruid = 0 then 
		select 0,0,0;
	else
		select TaksID, bFinished, CurCount from t_task where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_TradeData_Req` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_TradeData_Req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_TradeData_Req`(pUid_Goods bigint unsigned,pUid_Seller bigint unsigned,pName varchar(18),pPrice int unsigned, pGoodsCategory tinyint unsigned, pSubClass tinyint unsigned, pTime bigint unsigned, pGoodsData tinyblob)
BEGIN
	insert into t_trade(Uid_Goods,Uid_Seller, SellerName, Price, GoodsCategory, SubClass, Time, GoodsData) values(pUid_Goods,pUid_Seller, pName, pPrice, pGoodsCategory, pSubClass, pTime, pGoodsData);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Insert_XiuLianInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Insert_XiuLianInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Insert_XiuLianInfo`(pUid_User bigint unsigned,pAloneXLActor bigint unsigned,pAloneXLActor2 bigint unsigned,pAloneXLActor3 bigint unsigned,pAloneXLActor4 bigint unsigned,pTwoXLUidActor bigint unsigned,pTwoXLUidFriend  bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_xiulian where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_xiulian(Uid_User,AloneXLActor,AloneXLActor2,AloneXLActor3,AloneXLActor4,TwoXLUidActor,TwoXLUidFriend) values(pUid_User,pAloneXLActor,pAloneXLActor2,pAloneXLActor3,pAloneXLActor4,pTwoXLUidActor,pTwoXLUidFriend);
	end if;
	select Uid_User into t_uidUser from t_xiulian where Uid_User=pUid_User;
	if t_uidUser = 0 then
		select pUid_User,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'',0;
		select 0,0,0,0,0,0,0,0,0,0,'',0,0,'',0,0,0,0,0,0,0,0,0,0,0;
	else
		select * from t_xiulian where Uid_User = pUid_User;
		select * from t_xiulian_record where uidsource = pUid_User or uidFriend = pUid_User;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_OffLine_DeleteRes` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_OffLine_DeleteRes` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_OffLine_DeleteRes`(pUserID int unsigned, pMoney int ,pStone int, pTicket int)
BEGIN
	declare t_money int unsigned default 0;
	declare t_stone int unsigned default 0;
	declare t_ticket int unsigned default 0;
	select ActorMoney,ActorStone,ActorTicket into t_money,t_stone,t_ticket from t_actors where UserID = pUserID;
	if t_money < pMoney then
		set pMoney = t_money;
	end if;
	if t_stone < pStone then
		set pStone = t_stone;
	end if;
	if t_ticket < pTicket then
		set pTicket = t_ticket;
	end if;
	update t_actors set ActorMoney = pMoney, ActorTicket = pTicket, ActorStone = pStone where UserID = pUserID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_OffLine_RemoveGoods` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_OffLine_RemoveGoods` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_OffLine_RemoveGoods`(pUserID int unsigned,pGoodsID smallint unsigned,pGoodsNum smallint unsigned)
BEGIN
	declare t_uid bigint unsigned default 0;
	declare t_uidGoods bigint unsigned default 0;
	declare t_goodsNum smallint unsigned default 0;
	
	select uid into t_uid from t_actors where UserID = pUserID;
	if t_uid <> 0 then
		while pGoodsNum > 0 do
			set t_uidGoods = 0;
			select UidGoods,Number into t_uidGoods,t_goodsNum from t_goods where IDGoods = pGoodsID;
			if t_uidGoods = 0 then
				set pGoodsNum = 0;
			else
				if t_goodsNum > pGoodsNum then
					update t_goods set Number = t_goodsNum - pGoodsNum where UidGoods = t_uidGoods;
					set pGoodsNum = 0;
				else
					delete from t_goods where UidGoods = t_uidGoods;
					set pGoodsNum = pGoodsNum - t_goodsNum;
				end if;
			end if;
		end while;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_OpenServer_Init` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_OpenServer_Init` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_OpenServer_Init`()
BEGIN
	declare t_rankChallent int unsigned default 0;
	declare t_rankUser int unsigned default 0;
	select Rank into t_rankChallent from t_challengerank limit 0,1;
	select Rank into t_rankUser from t_userrank limit 0,1;
	if t_rankChallent = 0 then
		ALTER TABLE `t_challengerank` AUTO_INCREMENT=1;
	end if;
	if t_rankUser = 0 then
		ALTER TABLE `t_userrank` AUTO_INCREMENT=1;
	end if;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Save_ResBuildRecord` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Save_ResBuildRecord` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Save_ResBuildRecord`(pUidUser bigint unsigned, pBuildingType tinyint unsigned, pHappenTime int unsigned, pszTargetName varchar(18), pszHandleName varchar(18), pszContext varchar(50))
BEGIN
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdataSyndicate` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdataSyndicate` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdataSyndicate`(pSynID smallint unsigned,pSynName varchar(18),pUid_Leader bigint unsigned,pLeaderName varchar(18),pLevel tinyint unsigned,pExp int unsigned,pMemberNum smallint unsigned)
BEGIN
	update t_syndicate set SynName=pSynName,Uid_Leader=pUid_Leader,LeaderName=pLeaderName,Level=pLevel,Exp=pExp,MemberNum=pMemberNum where SynID=pSynID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdataSyndicateMember` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdataSyndicateMember` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdataSyndicateMember`(pUid_Member bigint unsigned,pPosition tinyint unsigned,pContribution int unsigned)
BEGIN
	update t_syndicatemember set Position = pPosition,Contribution=pContribution where Uid_User = pUid_Member;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateActorInfo2` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateActorInfo2` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateActorInfo2`(pUserID int unsigned,pName varchar(18),pLevel tinyint unsigned,pSpirit int,pShield int,pAvoid int,pActorExp int,pActorLayer int,pActorNimbus int,pActorAptitude int,pActorSex tinyint unsigned,puid bigint unsigned,pActorMoney int,pActorTicket int,pActorStone int,
							pActorFacade smallint unsigned,pActorBloodUp int,pDir tinyint unsigned,pptX smallint unsigned,pptY smallint unsigned,puidMaster bigint unsigned,pCityID smallint unsigned, pHonor int, pCredit int,pActorNimbusSpeed int,pGodSwordNimbus int,pUseFlag Binary(2),pVipLevel tinyint unsigned,pRecharge int)
BEGIN
	declare t_uid bigint unsigned default 0;
	select uid into t_uid from t_actors where uid=puid;
 	if t_uid = 0 then
		insert into t_actors values(pUserID,pName,pLevel,pSpirit,pShield,pAvoid,pActorExp,pActorLayer,pActorNimbus,pActorAptitude,pActorSex,puid,pActorMoney,pActorTicket,pActorStone,pActorFacade,pActorBloodUp,pDir,pptX,pptY,puidMaster,pCityID,pHonor,pCredit,pActorNimbusSpeed,pGodSwordNimbus,pUseFlag,pVipLevel,pRecharge);
	else
		update t_actors set Name=pName,Level=pLevel,Spirit=pSpirit,Shield=pShield,Avoid=pAvoid,ActorExp=pActorExp,ActorLayer=pActorLayer,ActorNimbus=pActorNimbus,ActorAptitude=pActorAptitude,ActorSex=pActorSex,ActorMoney=pActorMoney,ActorTicket=pActorTicket,ActorStone=pActorStone,ActorFacade=pActorFacade,
						ActorBloodUp=pActorBloodUp,nDir=pDir,ptX=pptX,ptY=pptY,uidMaster=puidMaster,CityID=pCityID,Honor=pHonor,Credit=pCredit,ActorNimbusSpeed=pActorNimbusSpeed,GodSwordNimbus=pGodSwordNimbus,UserFlag=pUseFlag,VipLevel=pVipLevel,Recharge=pRecharge   where uid=puid;
		update t_challengerank set level = pLevel,layer = pActorLayer,viplevel=pVipLevel where t_challengerank.uidUser=puid;
	 end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateBuildRecord` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateBuildRecord` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateBuildRecord`(pUid_User bigint unsigned,pRecordNum int unsigned,pBuildRecord blob(10240))
BEGIN
	delete from t_buildrecord where Uid_User = pUid_User;
	select Func_Update_BuildRecordInfo(pUid_User,pRecordNum,pBuildRecord);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateDouFaInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateDouFaInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_UpdateDouFaInfo`(pUserUid bigint unsigned, pGetHonorToday int, pLastGetHonorTime int unsigned,pJoinChallengeNum tinyint unsigned,pLastJoinChallengeTime int unsigned,pMaxChallengeNum smallint unsigned,pDouFaQieCuoEnemy varbinary(1024))
BEGIN
	declare t_uid bigint unsigned default 0;
	select UserUid into t_uid from t_doufa where UserUid = pUserUid;
	if t_uid = 0 then
		insert into t_doufa values(pUserUid, pGetHonorToday, pLastGetHonorTime,pJoinChallengeNum,pLastJoinChallengeTime,pMaxChallengeNum,pDouFaQieCuoEnemy);
	else
		update t_doufa set  GetHonorToday = pGetHonorToday,RecordTime=pLastGetHonorTime,JoinChallengeNum=pJoinChallengeNum,LastJoinChallengeTime=pLastJoinChallengeTime,MaxChallengeNumToday=pMaxChallengeNum,DouFaQieCuoEnemy=pDouFaQieCuoEnemy where UserUid = pUserUid;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateEquip` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateEquip` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_UpdateEquip`(pUid_User bigint unsigned, pUid_Goods bigint unsigned, pGoodsID smallint unsigned, pCreateTime int unsigned, pNumber tinyint unsigned, pBinded bool, pLocation tinyint unsigned, pGoodsData varbinary(30))
BEGIN
	declare t_goodsid smallint unsigned default 0;
	select IDGoods into t_goodsid from t_goods where Uid_User = pUid_User and PositionGoods = 1 and Location = pLocation for update;
	if t_goodsid = 0 then
		insert into t_goods values(pUid_User,1,pLocation,pUid_Goods,pGoodsID,pCreateTime,pNumber,pBinded,pGoodsData);
	else
		update t_goods set UidGoods = pUid_Goods,IDGoods = pGoodsID, CreateTime = pCreateTime,Number = pNumber,Binded = pBinded, BinData = pGoodsData where Uid_User = pUid_User and PositionGoods = 1 and Location = pLocation;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateFriendDataInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateFriendDataInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateFriendDataInfo`(pUid_User bigint unsigned, pUid_Friend bigint unsigned, pAddRelationNum int unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	declare t_relationNum1 int unsigned default 0;
	declare t_relationNum2 int unsigned default 0;
	
	select RelationNum into t_relationNum1 from t_friend where uid_User = pUid_User and uid_Friend = pUid_Friend;
	select RelationNum into t_relationNum2 from t_friend where uid_User = pUid_Friend and uid_Friend = pUid_User;
	
	if t_relationNum2 > t_relationNum1 then
		set t_relationNum1 = t_relationNum2;
	end if;
	set t_relationNum1 = t_relationNum1 + pAddRelationNum;
	update t_friend set RelationNum = t_relationNum1 where uid_User = pUid_User and uid_Friend = pUid_Friend;
	update t_friend set RelationNum = t_relationNum1 where uid_User = pUid_Friend and uid_Friend = pUid_User;
	select t_relationNum1;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateFuMoDongInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateFuMoDongInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateFuMoDongInfo`(pUid_User bigint unsigned,pEndOnHookTime int unsigned,pAccelNumOfDay int unsigned,pLastAccelTime int unsigned,pLevel int unsigned,pLastGiveExpTime int unsigned,pGiveExp int unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_fumodong where Uid_User=pUid_User;
	if t_uidUser=0 then
		insert into t_fumodong values(pUid_User,pEndOnHookTime,pAccelNumOfDay,pLastAccelTime,pLevel,pLastGiveExpTime,pGiveExp);
	else
		update t_fumodong set EndOnHookTime = pEndOnHookTime,AccelNumOfDay = pAccelNumOfDay,LastAccelTime = pLastAccelTime,Level = pLevel,LastGiveExpTime = pLastGiveExpTime, GiveExp = pGiveExp where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_UpdateTrainingHallInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_UpdateTrainingHallInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_UpdateTrainingHallInfo`(pUid_User bigint unsigned,pRemainNum smallint unsigned,pBeginTime int unsigned,pLastFinishTime int unsigned,pGetExp int, pStatus tinyint unsigned,pVipFinishTrainNum tinyint unsigned,pLastVipFinishTrainTime int unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_traininghall where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_traininghall values(pUid_User,pRemainNum,pBeginTime,pLastFinishTime, pGetExp,pStatus,pVipFinishTrainNum,pLastVipFinishTrainTime);
	else
		update t_traininghall set RemainNum=pRemainNum,BeginTime=pBeginTime,LastFinishTime=pLastFinishTime,GetExp=pGetExp,TrainStatus=pStatus,VipFinishTrainNum=pVipFinishTrainNum,LastVipFinishTrainTime=pLastVipFinishTrainTime where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_ActivityData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_ActivityData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_ActivityData`(puidActor bigint unsigned,pActivityID smallint unsigned,pbFinished tinyint unsigned,
                                         pbTakeAward tinyint unsigned,pActivityProgress int unsigned)
BEGIN
           declare t_uidUser bigint unsigned default 0;
           select uidActor into t_uidUser from t_activitydata where uidActor = puidActor and ActivityID = pActivityID for update;
           if t_uidUser = 0 then
              insert into t_activitydata(uidActor,ActivityID,bFinished,bTakeAward,ActivityProgress) value(puidActor,pActivityID,bFinished,pbTakeAward,pActivityProgress);
           else
              update t_activitydata set bFinished =pbFinished,bTakeAward=pbTakeAward,ActivityProgress=pActivityProgress where uidActor = puidActor and ActivityID = pActivityID;
           end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_ActivityPart` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_ActivityPart` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_ActivityPart`( pUidActor bigint unsigned, pSignInNumOfMonth tinyint unsigned,
                      pLastSignInTime int unsigned,pAwardHistory varbinary(1024),pOnLineAwardID smallint unsigned,
                       pLastOnLineAwardRestTime int unsigned,pbCanTakeOnlineAward tinyint,pOffLineNum tinyint,pNewPlayerGuideContext varchar(50))
BEGIN
             declare t_uidUser bigint unsigned default 0;
             select uidActor into t_uidUser from t_activity where uidActor = pUidActor for update ; 
             
            if t_uidUser =0 then
                insert into t_activity(uidActor,SignInNumOfMonth,LastSignInTime,AwardHistory,OnLineAwardID,LastOnLineAwardRestTime,bCanTakeOnlineAward,OffLineNum,NewPlayerGuideContext) 
                       value (pUidActor,pSignInNumOfMonth,pLastSignInTime,pAwardHistory,pOnLineAwardID,pLastOnLineAwardRestTime,bCanTakeOnlineAward,pOffLineNum,pNewPlayerGuideContext);
            else
                update t_activity set SignInNumOfMonth=pSignInNumOfMonth,LastSignInTime=pLastSignInTime,AwardHistory=pAwardHistory ,OnLineAwardID = pOnLineAwardID
                 ,LastOnLineAwardRestTime=pLastOnLineAwardRestTime,bCanTakeOnlineAward=pbCanTakeOnlineAward,OffLineNum=pOffLineNum,NewPlayerGuideContext=pNewPlayerGuideContext where uidActor = pUidActor;
            end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_AllUserLevelChallenge` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_AllUserLevelChallenge` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_AllUserLevelChallenge`()
BEGIN
	update t_challengerank inner join t_actors on t_challengerank.uidUser = t_actors.uid set t_challengerank.level = t_actors.Level;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_BuildingInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_BuildingInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_BuildingInfo`(pUid bigint unsigned,
                                                                          pBuildingType tinyint unsigned,
                                                                          pLastTakeResTime int unsigned,
									  puidOwner bigint unsigned,
                                                                          pOwnerNoTakeResNum int,
                                                                          pLastCollectTime int unsigned,
                                                                          pTotalCollectResOfDay int unsigned,
									  pbAutoTakeRes tinyint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_Building into t_uidUser from t_building where Uid_Building = pUid for update;
	
        if t_uidUser = 0 then
		insert into t_building values(pUid,pBuildingType,pLastTakeResTime,puidOwner,pOwnerNoTakeResNum,pLastCollectTime,pTotalCollectResOfDay,pbAutoTakeRes);
	else
		update t_building set BuildingType = pBuildingType,LastTakeResTime=pLastTakeResTime,Uid_Owner=puidOwner,OwnerNoTakeResNum=pOwnerNoTakeResNum,LastCollectTime= pLastCollectTime, TotalCollectResOfDay = pTotalCollectResOfDay, bAutoTakeRes = pbAutoTakeRes where Uid_Building=pUid;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_CDTimer` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_CDTimer` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_CDTimer`(pUidActor bigint unsigned, pCDTimerID smallint unsigned, pEndTime int unsigned)
BEGIN
	declare t_cdtimerID smallint unsigned default 0;
	
	select CDTimerID into t_cdtimerID from t_cdtimer where UidActor=pUidActor and CDTimerID=pCDTimerID for update;
	if t_cdtimerID = 0 then
		insert into t_cdtimer values(pUidActor, pCDTimerID, pEndTime);
	else
		update t_cdtimer set EndTime = pEndTime where UidActor=pUidActor and CDTimerID=pCDTimerID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_ChallengeRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_ChallengeRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_ChallengeRank`()
BEGIN
		truncate table t_challengerank;
		ALTER TABLE `t_challengerank` AUTO_INCREMENT=1;
		insert into t_challengerank(uidUser,Name,level,layer,titleID) select t_actors.uid,t_actors.Name,t_actors.Level,t_actors.ActorLayer,t_chengjiupart.ActiveTitleID 
			from t_actors,t_chengjiupart where t_actors.uid = t_chengjiupart.UserUID and uidMaster=0 order by Level desc ,ActorExp desc;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_ChengJiu` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_ChengJiu` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_ChengJiu`(pUserUID bigint unsigned,pChengJiuID smallint unsigned,pCurCount smallint unsigned,pFinishTime int unsigned)
BEGIN
	declare t_uid bigint unsigned default 0;
	select UserUID into t_uid from t_chengjiu where UserUID = pUserUID and ChengJiuID = pChengJiuID;
	
	if t_uid = 0 then
		insert into t_chengjiu values(pUserUID,pChengJiuID,pCurCount,pFinishTime);
	else
		update t_chengjiu set CurCount = pCurCount, FinishTime = pFinishTime where UserUID = pUserUID and ChengJiuID = pChengJiuID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_ChengJiuPart` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_ChengJiuPart` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_ChengJiuPart`(pUserUID bigint unsigned,pChengJiuPoint int,pGetChengJiuNum int,pActiveTitleID tinyint unsigned)
BEGIN
	declare t_uid bigint unsigned default 0;
	select UserUID into t_uid from t_chengjiupart where UserUID = pUserUID for update;
	if t_uid = 0 then
		insert into t_chengjiupart values(pUserUID,pChengJiuPoint,pGetChengJiuNum,pActiveTitleID);
	else
		update t_chengjiupart set ChengJiuPoint = pChengJiuPoint,GetChengJiuNum = pGetChengJiuNum,ActiveTitleID = pActiveTitleID where UserUID = pUserUID;
		update t_challengerank set titleID=pActiveTitleID where uidUser=pUserUID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_CombatInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_CombatInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_CombatInfo`(pUid_User bigint unsigned,puidLineup1 bigint unsigned,puidLineup2 bigint unsigned,puidLineup3 bigint unsigned,puidLineup4 bigint unsigned,
									puidLineup5 bigint unsigned, puidLineup6 bigint unsigned,puidLineup7 bigint unsigned,puidLineup8 bigint unsigned,puidLineup9 bigint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_combat where Uid_User = pUid_User;
	if t_uidUser = 0 then
		insert into t_combat values(pUid_User,puidLineup1,puidLineup2,puidLineup3,puidLineup4,puidLineup5,puidLineup6,puidLineup7,puidLineup8,puidLineup9);
	else
		update t_combat set  uidLineup1 = puidLineup1,uidLineup2=puidLineup2,uidLineup3=puidLineup3,uidLineup4 = puidLineup4,
				uidLineup5 = puidLineup5,uidLineup6 = puidLineup6,uidLineup7 = puidLineup7,uidLineup8 = puidLineup8,uidLineup9 = puidLineup9 where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_EquipPanelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_EquipPanelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_EquipPanelInfo`(pUid_User bigint unsigned,pEquipNum smallint unsigned,pEquipGoodsData blob(10240))
BEGIN
	-- 闁稿繐鐗嗛崹褰掓⒔閵堝洤璐熼悗纭呮硾濠�亪寮悧鍫濈ウ閹煎瓨鎸风粭鍌炴儍閸曨噮妫呭?
	delete from t_goods where Uid_User = pUid_User and PositionGoods = 1;
	-- 闁哄洤鐡ㄩ弻濡昣goods閻炴稏鍔庡▓鎴﹀极閻楀牆绁�                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
	select Func_Update_GoodsInfo(1,pUid_User,pEquipNum,pEquipGoodsData); 
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_GatherGodHouseInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_GatherGodHouseInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_GatherGodHouseInfo`(pUid_User bigint unsigned,pLastFlushEmployTime int unsigned,pIdEmployee1 int unsigned,pIdEmployee2 int  unsigned,pIdEmployee3 int unsigned,
							pIdEmployee4 int unsigned,pLastFlushMagicTime int unsigned,pIdMagicBook1 smallint unsigned,pIdMagicBook2 smallint unsigned,pIdMagicBook3 smallint unsigned,pIdMagicBook4 smallint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_gathergodhouse where Uid_User=pUid_User for update;
	if t_uidUser = 0 then
		insert into t_gathergodhouse values(pUid_User,pLastFlushEmployTime,pIdEmployee1,pIdEmployee2,pIdEmployee3,pIdEmployee4,pLastFlushMagicTime,pIdMagicBook1,pIdMagicBook2,pIdMagicBook3,pIdMagicBook4);
	else
		update t_gathergodhouse set LastFlushEmployTime=pLastFlushEmployTime,IdEmployee1=pIdEmployee1,IdEmployee2=pIdEmployee2,IdEmployee3=pIdEmployee3,IdEmployee4=pIdEmployee4,LastFlushMagicTime=pLastFlushMagicTime,idMagicBook1=pIdMagicBook1,idMagicBook2=pIdMagicBook2,idMagicBook3=pIdMagicBook3,idMagicBook4=pIdMagicBook4 where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_GodSwordShopInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_GodSwordShopInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_GodSwordShopInfo`(pUid_User bigint unsigned,pLastFlushTime bigint unsigned,pIdGodSword1 smallint unsigned,pIdGodSword2 smallint unsigned,pIdGodSword3 smallint unsigned,pIdGodSword4 smallint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_godswordshop where Uid_User=pUid_User for update;
	if t_uidUser = 0 then
		insert into t_godswordshop values(pUid_User,pLastFlushTime,pIdGodSword1,pIdGodSword2,pIdGodSword3,pIdGodSword4);
	else
		update t_godswordshop set LastFlushTime=pLastFlushTime,IdGodSword1=pIdGodSword1,IdGodSword2=pIdGodSword2,IdGodSword3=pIdGodSword3,IdGodSword4=pIdGodSword4 where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_Goods` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_Goods` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_Goods`(puidUser bigint unsigned, puidGoods bigint unsigned, pGoodsID smallint unsigned, pCreateTime int unsigned, pNumber tinyint unsigned, pBinded bool, pLocation tinyint unsigned,pPos tinyint unsigned,pGoodsData varbinary(30))
BEGIN
	declare t_goodsID smallint unsigned default 0;
	select IDGoods into t_goodsID from t_goods where Uid_User = puidUser and PositionGoods = pPos and UidGoods = puidGoods for update;
	if t_goodsID  = 0 then
		insert into t_goods values(puidUser, pPos, pLocation, puidGoods, pGoodsID, pCreateTime, pNumber, pBinded,pGoodsData);
	else
		update t_goods set CreateTime = pCreateTime, Number = pNumber, Binded = pBinded, BinData = pGoodsData,Location = pLocation, PositionGoods = pPos where  Uid_User = puidUser and PositionGoods = pPos and UidGoods = puidGoods;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_MagicPanelInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_MagicPanelInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_MagicPanelInfo`(pUid_User bigint unsigned,pMagicPanelData blob(10240))
BEGIN
	-- 閸忓牆鍨归梽銈囧负鐎硅泛婀弫鐗堝祦鎼存挷鑵戦惃鍕殶閹�
	delete from t_magic where Uid_User = pUid_User;
	-- 閸愬秵濡搁悳鈺侇啀闊偂绗傞惃鍕Η閼宠棄鍙忛弴瀛樻煀閸掔増鏆熼幑顔肩氨
	select Func_Update_MagicPanelInfo(pUid_User,pMagicPanelData);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_MailData_Req` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_MailData_Req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_MailData_Req`(pMailID int unsigned, pStone int unsigned, pMoney int unsigned, pTicket int unsigned, pSynID smallint unsigned,pbRead tinyint unsigned,pGoodsData varbinary(1024))
BEGIN
	update t_mail set stone = pStone, money = pMoney, ticket = pTicket, SynID=pSynID, bRead = pbRead,GoodsData = pGoodsData where MailID = pMailID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_PacketGoods` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_PacketGoods` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_PacketGoods`(puidUser bigint unsigned, puidGoods bigint unsigned, pGoodsID smallint unsigned, pCreateTime int unsigned, pNumber tinyint unsigned, pBinded bool, pGoodsData varbinary(30))
BEGIN
	declare t_goodsID smallint unsigned default 0;
	select IDGoods into t_goodsID from t_goods where Uid_User = puidUser and PositionGoods = 0 and UidGoods = puidGoods for update;
	if t_goodsID  = 0 then
		insert into t_goods values(puidUser, 0, 0, puidGoods, pGoodsID, pCreateTime, pNumber, pBinded, pGoodsData);
	else
		update t_goods set CreateTime = pCreateTime, Number = pNumber, Binded = pBinded, BinData = pGoodsData;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_PacketInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_PacketInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_PacketInfo`(pUid_User bigint unsigned, pCapacity smallint unsigned, pGoodsNum smallint unsigned,pPacketGoodsData blob(10240))
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_packet where Uid_User = pUid_User ; -- for update;
	if t_uidUser = 0 then
		insert into t_packet values(pUid_User, pCapacity, pGoodsNum);
	else
		update t_packet set Capacity = pCapacity,GoodsNum = pGoodsNum where Uid_User = pUid_User;
	end if;
	
	-- 閸忓牆鍨归梽銈囧负鐎硅泛婀弫鐗堝祦鎼存挷绗傞惃鍕⒖閸�
	delete from t_goods where Uid_User = pUid_User and PositionGoods = 0;
	-- 閺囧瓨鏌妕_goods鐞涖劎娈戦弫鐗堝祦
	select Func_Update_GoodsInfo (0,pUid_User,pGoodsNum,pPacketGoodsData);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_PacketPart` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_PacketPart` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_PacketPart`(puidUser bigint unsigned, pCapacity smallint unsigned, pGoodsNum smallint unsigned)
BEGIN
	declare t_UidUser bigint unsigned default 0;
	select Uid_User into t_UidUser from t_packet where Uid_User = puidUser for update;
	
	if t_UidUser = 0 then
		insert into t_packet values(puidUser,pCapacity,pGoodsNum);
	else
		update t_packet set Capacity = pCapacity, GoodsNum = pGoodsNum where Uid_User = puidUser;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_ShopMallCnfg` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_ShopMallCnfg` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_ShopMallCnfg`(pID int unsigned, pLeftFen int)
BEGIN
	update t_shopmall set leftFen = pLeftFen where id = pID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_StatusInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_StatusInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_StatusInfo`(pUid_User bigint unsigned, pStatusID smallint unsigned, pEndStatusTime int unsigned,pUidCreator bigint unsigned)
BEGIN
	declare t_statusID smallint unsigned default 0;
	select StatusID into t_statusID from t_status where Uid_User = pUid_User and StatusID = pStatusID for update;
	
	if t_statusID = 0 then
		insert into t_status values(pUid_User, pStatusID, pEndStatusTime, pUidCreator);
	else
		update t_status set EndStatusTime = pEndStatusTime, UidCreator = pUidCreator where Uid_User = pUid_User and StatusID = pStatusID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_SynMagicInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_SynMagicInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_SynMagicInfo`(pUid_User bigint unsigned, pSynMagicData blob(10240))
BEGIN
	-- 閸忓牆鍨归梽銈囧负鐎硅泛婀弫鐗堝祦鎼存挷鑵戦惃鍕殶閹�
	delete from t_syndicatemagic where Uid_User = pUid_User;
	-- 閸愬秵濡搁悳鈺侇啀闊偂绗傞惃鍕簻濞茬偓濡ч懗钘夊弿閺囧瓨鏌婇崚鐗堟殶閹诡喖绨�
	select Func_Update_SynMagicInfo(pUid_User,pSynMagicData);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_TaskInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_TaskInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_TaskInfo`(pUid_User bigint unsigned, pTask_Num tinyint unsigned, pTaskData blob(10240))
BEGIN
	delete from t_task where Uid_User = pUid_User;
	select Func_Update_TaskInfo(pUid_User, pTask_Num, pTaskData);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_TaskPartInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_TaskPartInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_TaskPartInfo`(pUid_User bigint unsigned, pTaskHistory Binary(32), pLastUpdateTime int unsigned,pNotSaveNum tinyint unsigned, pTaskStatus tinyint unsigned,pTaskData varbinary(2048))
BEGIN
	declare t_num int default 0;
	select count(*) into t_num from t_taskpart where Uid_User = pUid_User;
	if t_num = 0 then
		insert into t_taskpart values(pUid_User, pTaskHistory, pLastUpdateTime,pNotSaveNum, pTaskStatus,pTaskData);
	else
		update t_taskpart set TaskHistory = pTaskHistory, LastUpdateTime = pLastUpdateTime, NotSaveNum = pNotSaveNum, TaskStatus =pTaskStatus, NotSaveTaskID = pTaskData  where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_UserCDTimer_Req` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_UserCDTimer_Req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_UserCDTimer_Req`(pUidActor bigint unsigned, pCDTimerID smallint unsigned, pEndTime int unsigned)
BEGIN
	declare t_cdtimerID smallint unsigned default 0;
	
	select CDTimerID into t_cdtimerID from t_cdtimer where UidActor=pUidActor and CDTimerID=pCDTimerID for update;
	if t_cdtimerID = 0 then
		insert into t_cdtimer values(pUidActor, pCDTimerID, pEndTime);
	else
		update t_cdtimer set EndTime = pEndTime where UidActor=pUidActor and CDTimerID=pCDTimerID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_UserRank` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_UserRank` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Update_UserRank`()
BEGIN
	truncate table t_userrank;
	 ALTER TABLE `t_userrank` AUTO_INCREMENT=1;
        insert into t_userrank(uidUser,Name,Level,Exp,Layer,viplevel) select uid,Name,Level,ActorExp,ActorLayer,VipLevel from t_actors where uidMaster=0 order by Level desc ,ActorExp desc;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_XiuLianInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_XiuLianInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_XiuLianInfo`(pUid_User bigint unsigned,pAloneXiuLianState tinyint unsigned,pAloneHours tinyint unsigned,pAloneLastGetNimbusTime int unsigned,
							pAloneEndTime int unsigned,pAloneXLActor bigint unsigned,pAloneXLActor2 bigint unsigned,
							pAloneXLActor3 bigint unsigned,pAloneXLActor4 bigint unsigned,pAloneXLActor5 bigint unsigned,pAloneXLActor6 bigint unsigned,pAloneXLActor7 bigint unsigned,pAloneXLActor8 bigint unsigned,
							pAloneXLGetNimbus int,pAloneXLGetNimbus2 int,pAloneXLGetNimbus3 int, pAloneXLGetNimbus4 int,pAloneXLGetNimbus5 int,pAloneXLGetNimbus6 int,pAloneXLGetNimbus7 int,pAloneXLGetNimbus8 int,
							pGetGodSwordNimbus int unsigned, pTwoXiuLianState tinyint unsigned,pTwoXLHours tinyint unsigned,pTwoXLLastGetNimbusTime int unsigned,
							pTwoXLTotalNimbus int,pTwoXLEndTime int unsigned,pTwoXLUidActor bigint unsigned,pTwoXLUidFriend bigint unsigned,pFriendFacade int unsigned,pFriendName varchar(18),pMagicXLSeq int unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from  t_xiulian where Uid_User = pUid_User;
	if t_uidUser = 0 then
		insert into t_xiulian values(pUid_User,pAloneXiuLianState,pAloneHours,pAloneLastGetNimbusTime,pAloneEndTime,pAloneXLActor,pAloneXLActor2,pAloneXLActor3,pAloneXLActor4,pAloneXLActor5,pAloneXLActor6,pAloneXLActor7,pAloneXLActor8,pGetGodSwordNimbus,pAloneXLGetNimbus,
						pAloneXLGetNimbus2,pAloneXLGetNimbus3,pAloneXLGetNimbus4,pAloneXLGetNimbus5,pAloneXLGetNimbus6,pAloneXLGetNimbus7,pAloneXLGetNimbus8,pTwoXiuLianState,pTwoXLHours,pTwoXLLastGetNimbusTime,pTwoXLTotalNimbus,pTwoXLEndTime,pTwoXLUidActor,pTwoXLUidFriend,pFriendFacade,pFriendName,pMagicXLSeq);
	else
		update t_xiulian set AloneXiuLianState=pAloneXiuLianState,AloneHours=pAloneHours,AloneLastGetNimbusTime=pAloneLastGetNimbusTime,AloneEndTime=pAloneEndTime,AloneXLActor=pAloneXLActor,
					AloneXLActor2=pAloneXLActor2,AloneXLActor3=pAloneXLActor3,AloneXLActor4=pAloneXLActor4,AloneXLActor5 = pAloneXLActor5,AloneXLActor6=pAloneXLActor6,AloneXLActor7=pAloneXLActor7,AloneXLActor8=pAloneXLActor8,GetGodSwordNimbus =pGetGodSwordNimbus,AloneXLGetNimbus=pAloneXLGetNimbus,AloneXLGetNimbus2=pAloneXLGetNimbus2,AloneXLGetNimbus3=pAloneXLGetNimbus3,
					AloneXLGetNimbus4=pAloneXLGetNimbus4, AloneXLGetNimbus5=pAloneXLGetNimbus5,AloneXLGetNimbus6=pAloneXLGetNimbus6,AloneXLGetNimbus7=pAloneXLGetNimbus7,AloneXLGetNimbus8=pAloneXLGetNimbus8,TwoXiuLianState=pTwoXiuLianState,TwoXLHours=pTwoXLHours,TwoXLLastGetNimbusTime=pTwoXLLastGetNimbusTime,
					TwoXLTotalNimbus = pTwoXLTotalNimbus,TwoXLEndTime=pTwoXLEndTime,TwoXLUidActor=pTwoXLUidActor,TwoXLUidFriend=pTwoXLUidFriend,FriendFacade=pFriendFacade,FriendName=pFriendName,MagicXLSeq=pMagicXLSeq  where Uid_User = pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Update_XiuLianRecord` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Update_XiuLianRecord` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Update_XiuLianRecord`(pXiuLianType tinyint unsigned,pXiuLianState tinyint unsigned,pHours tinyint unsigned,pMode tinyint unsigned,
											pLastGetNimbusTime int unsigned,pTotalNimbus int,pEndTime int unsigned,pAskSeq int unsigned,pUidSource bigint unsigned,
											pUidSourceActor bigint unsigned,pSourceName varchar(18),pUidFriend bigint unsigned,pUidFriendActor bigint unsigned,
											pFriendName varchar(18),pMagicID smallint unsigned,pAskTime int unsigned,pFriendTotalNimbus int,
											pSourceNotTakeNimbus int,pFriendNotTakeNimbus int,pStudyMagic bool, pFriendLastGetNimbusTime int unsigned, pSourceNimbusSpeed int unsigned,
											pFriendNimbusSpeed int unsigned,pSourceLayer int unsigned,pFriendLayer int unsigned)
BEGIN
	declare t_num int default 0;
	select count(*) into t_num from t_xiulian_record where UidSource = pUidSource or UidSource=pUidFriend or UidFriend=pUidFriend or UidFriend=pUidSource;
	if t_num=0 then
		insert into t_xiulian_record values(pXiuLianType,pXiuLianState,pHours,pMode,pLastGetNimbusTime,pTotalNimbus,pEndTime,pAskSeq,pUidSource,pUidSourceActor,pSourceName,pUidFriend,pUidFriendActor,
			pFriendName,pMagicID,pAskTime,pFriendTotalNimbus,pSourceNotTakeNimbus,pFriendNotTakeNimbus,pStudyMagic,pFriendLastGetNimbusTime,pSourceNimbusSpeed,pFriendNimbusSpeed,pSourceLayer,pFriendLayer);
	else
		update t_xiulian_record set XiuLianType = pXiuLianType,XiuLianState=pXiuLianState,Hours=pHours,Mode=pMode,LastGetNimbusTime=pLastGetNimbusTime,TotalNimbus=pTotalNimbus,EndTime=pEndTime,AskSeq=pAskSeq,
		UidSource=pUidSource,UidSourceActor=pUidSourceActor,SourceName=pSourceName,UidFriend=pUidFriend,UidFriendActor=pUidFriendActor,FriendName=pFriendName,MagicID=pMagicID,AskTime=pAskTime,
		FriendTotalNimbus=pFriendTotalNimbus,SourceNotTakeNimbus=pSourceNotTakeNimbus,FriendNotTakeNimbus=pFriendNotTakeNimbus,bStudyMagic=pStudyMagic,FriendLastGetNimbusTime=pFriendLastGetNimbusTime,
		SourceNimbusSpeed=pSourceNimbusSpeed,FriendNimbusSpeed=pFriendNimbusSpeed,SourceLayer=pSourceLayer,FriendLayer=pFriendLayer where UidSource = pUidSource or UidSource=pUidFriend or UidFriend=pUidFriend or UidFriend=pUidSource;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_WriteAllUserSysMail` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_WriteAllUserSysMail` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_WriteAllUserSysMail`(pUidSendUser bigint unsigned,pNameSendUser varchar(18), pMailType tinyint unsigned,pStone int unsigned,
								pMoney int unsigned,pTicket int unsigned,pThemeText varchar(50), pContentText varchar(300), 
								pTime bigint unsigned,pGoodsData varbinary(1024))
BEGIN
	insert into t_mail(uid_User,uid_SendUser,Name_SendUser,Mail_Type,stone,money,ticket,SynID,ThemeText,ContentText,Time,GoodsData)
		select uid,pUidSendUser,pNameSendUser,pMailType,pStone,pMoney,pTicket,0,pThemeText,pContentText,pTime,pGoodsData from t_actors where t_actors.uidMaster = 0;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_Update_BasicFuBenInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_Update_BasicFuBenInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_Update_BasicFuBenInfo`(pUid_User bigint unsigned,pLastFreeEnterFuBenTime int unsigned,pLastStoneEnterFuBenTime int unsigned,pLastSynWelfareEnterFuBenTime int unsigned,pFreeEnterFuBenNum smallint unsigned,pSynWelfareFuBenNum smallint unsigned,pCostStoneEnterFuBenNum smallint unsigned,
							pFuBenNum tinyint unsigned,pLastEnterFuBenGodSword int unsigned,pEnterSynFuBenNum smallint unsigned,pLastEnterSynFuBenTime int unsigned,pEnterGodSwordWorldNum smallint unsigned,pLastVipEnterFuBenTime int unsigned,pVipEnterFuBenNum smallint unsigned,pFinishedFuBen binary(32))
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_fubenbasic where Uid_User=pUid_User;
	if t_uidUser = 0 then
		insert into t_fubenbasic values(pUid_User,pLastFreeEnterFuBenTime,pLastStoneEnterFuBenTime,pLastSynWelfareEnterFuBenTime,pFreeEnterFuBenNum,pSynWelfareFuBenNum,pCostStoneEnterFuBenNum,pFuBenNum,pLastEnterFuBenGodSword,pEnterSynFuBenNum,pLastEnterSynFuBenTime,pEnterGodSwordWorldNum,pLastVipEnterFuBenTime,pVipEnterFuBenNum,pFinishedFuBen);
	else
		update t_fubenbasic set LastFreeEnterFuBenTime=pLastFreeEnterFuBenTime,LastStoneEnterFuBenTime=pLastStoneEnterFuBenTime,
		LastSynWelfareEnterFuBenTime=pLastSynWelfareEnterFuBenTime,
                  FreeEnterFuBenNum=pFreeEnterFuBenNum,SynWelfareEnterFuBenNum = pSynWelfareFuBenNum,
                 CostStoneEnterFuBenNum=pCostStoneEnterFuBenNum,FuBenNum=pFuBenNum,LastEnterFuBenGodSword=pLastEnterFuBenGodSword,
		EnterSynFuBenNum=pEnterSynFuBenNum,LastEnterSynFuBenTime=pLastEnterSynFuBenTime,EnterGodSwordWorldNum=pEnterGodSwordWorldNum,
		LastVipEnterFuBenTime=pLastVipEnterFuBenTime,VipEnterFuBenNum=pVipEnterFuBenNum,FinishedTask=pFinishedFuBen where Uid_User=pUid_User;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_Update_FuBenProgressInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_Update_FuBenProgressInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_Update_FuBenProgressInfo`(pUid_User bigint unsigned,pFuBenID tinyint unsigned,pLevel tinyint unsigned,pKillMonsterNum tinyint unsigned)
BEGIN
	declare t_uidUser bigint unsigned default 0;
	select Uid_User into t_uidUser from t_fubenprogress where Uid_User=pUid_User and FuBenID=pFuBenID ;
	if t_uidUser = 0 then
		insert into t_fubenprogress values(pUid_User,pFuBenID,pLevel,pKillMonsterNum);
	else
               if pLevel=1 and pKillMonsterNum = 0 then
                   delete from t_fubenprogress where Uid_User=pUid_User and FuBenID=pFuBenID ;
               else
		update t_fubenprogress set Level=pLevel,KillMonsterNum=pKillMonsterNum where Uid_User=pUid_User and FuBenID=pFuBenID;
               end if;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `Test` */

/*!50003 DROP PROCEDURE IF EXISTS  `Test` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `Test`()
BEGIN
	declare t_num int default 0;
	 select  count(*) from (select * from t_syndicate order by SynID limit 10,60) as t_sdfs;
	-- (select * from t_syndicate order by SynID limit 10,60) as t_sdfs;
	-- select t_num;
	select * from t_syndicate order by SynID limit 10,60;
	-- select found_rows();
    END */$$
DELIMITER ;

/* Procedure structure for procedure `tryproc` */

/*!50003 DROP PROCEDURE IF EXISTS  `tryproc` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `tryproc`(pValue int unsigned)
BEGIN
	declare t_binary varbinary(100);
	declare t_Tmpbinary varbinary(100);
	declare t_value int unsigned default 0;
	set t_binary = cast(pValue as binary);
	set t_Tmpbinary = SUBSTRING(t_binary,1,4);
	-- set t_binary = pValue;
	set t_value = cast(t_Tmpbinary as unsigned);
	-- set t_value = t_binary;
	select t_value;
    END */$$
DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
