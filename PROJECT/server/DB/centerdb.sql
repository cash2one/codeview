锘�*
SQLyog Enterprise - MySQL GUI v7.14 
MySQL - 5.5.19 : Database - centerdb
*********************************************************************
*/
/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

CREATE DATABASE /*!32312 IF NOT EXISTS*/`centerdb` /*!40100 DEFAULT CHARACTER SET gbk COLLATE gbk_bin */;

USE `centerdb`;

/*Table structure for table `groups` */

DROP TABLE IF EXISTS `groups`;

CREATE TABLE `groups` (
  `group_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_name` varchar(50) NOT NULL,
  `group_power` varchar(250) DEFAULT NULL COMMENT '缁勬潈闄�,
  `system` tinyint(2) NOT NULL DEFAULT '0' COMMENT '鏄惁涓虹郴缁熻嚜甯︾粍',
  PRIMARY KEY (`group_id`)
) ENGINE=MyISAM AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COMMENT='鍚庡彴缁勮〃';

/*Table structure for table `module` */

DROP TABLE IF EXISTS `module`;

CREATE TABLE `module` (
  `id` int(11) NOT NULL,
  `name` varchar(50) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `purview` */

DROP TABLE IF EXISTS `purview`;

CREATE TABLE `purview` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `module` int(10) NOT NULL,
  `url` varchar(50) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=43 DEFAULT CHARSET=utf8;

/*Table structure for table `stonedeallog` */

DROP TABLE IF EXISTS `stonedeallog`;

CREATE TABLE `stonedeallog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `receiveUserid` int(11) NOT NULL,
  `executeUserid` int(11) NOT NULL,
  `time` datetime NOT NULL,
  `xianstone_num` int(11) NOT NULL DEFAULT '0',
  `lingstone_num` int(11) NOT NULL DEFAULT '0',
  `liquan_num` int(11) NOT NULL DEFAULT '0',
  `remark` varchar(250) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=28 DEFAULT CHARSET=utf8;

/*Table structure for table `t_allowenter` */

DROP TABLE IF EXISTS `t_allowenter`;

CREATE TABLE `t_allowenter` (
  `UserID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`UserID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_bangpaiposition` */

DROP TABLE IF EXISTS `t_bangpaiposition`;

CREATE TABLE `t_bangpaiposition` (
  `id` smallint(5) NOT NULL,
  `name` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_chengjiucnfg` */

DROP TABLE IF EXISTS `t_chengjiucnfg`;

CREATE TABLE `t_chengjiucnfg` (
  `ChengJiuID` int(10) unsigned NOT NULL,
  `ChengJiuName` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `ChengJiuPoint` int(5) DEFAULT NULL,
  PRIMARY KEY (`ChengJiuID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_controlcmd` */

DROP TABLE IF EXISTS `t_controlcmd`;

CREATE TABLE `t_controlcmd` (
  `UserID` int(10) unsigned NOT NULL,
  `Permission` char(100) COLLATE gbk_bin NOT NULL DEFAULT '""',
  PRIMARY KEY (`UserID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_controlcmdcnfg` */

DROP TABLE IF EXISTS `t_controlcmdcnfg`;

CREATE TABLE `t_controlcmdcnfg` (
  `cmdid` smallint(6) DEFAULT NULL,
  `cmdcode` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `cmddescription` varchar(200) COLLATE gbk_bin DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_donttalk` */

DROP TABLE IF EXISTS `t_donttalk`;

CREATE TABLE `t_donttalk` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛ID',
  `donttalkbegin` datetime DEFAULT NULL COMMENT '琚瑷�紑濮嬫椂闂�,
  `donttalkend` datetime DEFAULT NULL COMMENT '琚瑷�粨鏉熸椂闂�,
  `serverid` smallint(5) unsigned DEFAULT NULL COMMENT '琚瑷�殑鏈嶅姟鍣紝涓洪浂琛ㄧず鎵�湁鏈嶅姟鍣�,
  `donttalkReason` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '琚瑷�師鍥�,
  `donttalkOperatorID` int(10) unsigned DEFAULT NULL COMMENT '绂佽█鎿嶄綔鍛�,
  `donttalkOperateTime` datetime DEFAULT NULL COMMENT '璁剧疆绂佽█鎿嶄綔鏃堕棿',
  `removetime` datetime DEFAULT NULL COMMENT '瑙ｉ櫎绂佽█鏃堕棿',
  `removeOperatorID` int(10) unsigned DEFAULT NULL COMMENT '瑙ｉ櫎绂佽█鎿嶄綔鍛�,
  `removereason` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '瑙ｉ櫎鍘熷洜',
  `removeoperatetime` datetime DEFAULT NULL COMMENT '瑙ｉ櫎绂佽█鎿嶄綔鏃堕棿',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_gamecnfg` */

DROP TABLE IF EXISTS `t_gamecnfg`;

CREATE TABLE `t_gamecnfg` (
  `TotalChengJiuPoint` int(11) DEFAULT NULL COMMENT '鎬绘垚灏辩偣'
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_gamegoodslog` */

DROP TABLE IF EXISTS `t_gamegoodslog`;

CREATE TABLE `t_gamegoodslog` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `useid` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL COMMENT '绫诲瀷 0:鑾峰緱 1:澶卞幓',
  `chanel` tinyint(3) unsigned NOT NULL COMMENT '娓犻亾 0:璐拱 1:鎺夎惤 2:閭欢 3:浣跨敤 4:涓㈠純 5:鍑哄敭 6:瀵勫敭 7:鍚堟垚 8:寮哄寲 9:绯荤粺鎵ｉ櫎鐗╁搧 10:绯荤粺鐩存帴缁欎簣 11:浠诲姟 12:娉曞疂瀛曡偛鐗�13:娉曞畾涓栫晫 14:娲诲姩妯″潡 15:GM鍛戒护 16:娓告垙鍏跺畠',
  `goodsid` smallint(5) unsigned NOT NULL COMMENT '鐗╁搧id',
  `goodsuid` bigint(20) unsigned NOT NULL COMMENT '鐗╁搧uid',
  `goodsnum` smallint(5) unsigned NOT NULL COMMENT '鐗╁搧鏁伴噺',
  `time` datetime NOT NULL COMMENT '鏃堕棿',
  `descript` varchar(300) COLLATE gbk_bin NOT NULL COMMENT '璇存槑',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=408 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_godstonelog` */

DROP TABLE IF EXISTS `t_godstonelog`;

CREATE TABLE `t_godstonelog` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛id',
  `type` tinyint(3) unsigned DEFAULT NULL COMMENT '绫诲瀷:0鑾峰緱,1娑堣垂',
  `chanel` tinyint(4) DEFAULT NULL COMMENT '娓犻亾:0鍏呭�,1娓告垙,2缃戠,3閭欢',
  `serverid` smallint(6) DEFAULT NULL COMMENT '鏈嶅姟鍣╥d',
  `happentime` datetime DEFAULT NULL COMMENT '鍙戠敓鏃堕棿',
  `vargodstone` int(11) DEFAULT NULL COMMENT '鍙樺姩浠欑煶',
  `aftergodstone` int(11) DEFAULT NULL COMMENT '鍙樺姩鍚庝粰鐭�,
  `description` varchar(300) COLLATE gbk_bin DEFAULT NULL COMMENT '璇存槑',
  `associateid` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '鍏宠仈ID',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=507 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_goodcnfg` */

DROP TABLE IF EXISTS `t_goodcnfg`;

CREATE TABLE `t_goodcnfg` (
  `id` int(11) NOT NULL COMMENT '鐗╁搧ID',
  `name` varchar(256) COLLATE gbk_bin DEFAULT NULL COMMENT '鐗╁搧鍚嶇О',
  `description` varchar(1000) COLLATE gbk_bin DEFAULT NULL,
  `level` smallint(3) DEFAULT NULL,
  `resourceid` int(11) DEFAULT NULL COMMENT '璧勬簮ID',
  `category` smallint(6) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_goods` */

DROP TABLE IF EXISTS `t_goods`;

CREATE TABLE `t_goods` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '璁板綍ID',
  `OperateType` tinyint(3) unsigned DEFAULT NULL COMMENT '鎿嶄綔绫诲瀷:0澧炲姞,1鎵ｉ櫎',
  `GoodsID` smallint(5) unsigned DEFAULT NULL COMMENT '鐗╁搧ID',
  `GoodsName` varchar(18) COLLATE gbk_bin DEFAULT NULL COMMENT '鐗╁搧鍚嶇О',
  `Number` smallint(5) unsigned DEFAULT NULL COMMENT '鐗╁搧鏁伴噺',
  `ServerID` smallint(5) unsigned DEFAULT NULL COMMENT '鏈嶅姟鍣↖D',
  `TargetType` tinyint(4) DEFAULT NULL COMMENT '鐩爣绫诲瀷:0鎵�湁鐢ㄦ埛,1鎸囧畾鐜╁',
  `TargetList` varchar(1024) COLLATE gbk_bin DEFAULT NULL COMMENT '鐩爣鍒楄〃锛岄�鍙峰垎闅�,
  `OperatorID` int(10) unsigned DEFAULT NULL COMMENT '鎿嶄綔鍛業D',
  `OperateTime` datetime DEFAULT NULL COMMENT '鎿嶄綔鏃堕棿',
  `Reason` varchar(500) COLLATE gbk_bin DEFAULT NULL COMMENT '鎿嶄綔鍘熷洜',
  `MailID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `good_mailID` (`MailID`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_goodslog` */

DROP TABLE IF EXISTS `t_goodslog`;

CREATE TABLE `t_goodslog` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
  `OperateType` tinyint(3) unsigned DEFAULT NULL,
  `GoodsID` smallint(5) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛id',
  `GoodsName` varchar(18) COLLATE gbk_bin DEFAULT NULL COMMENT '绫诲瀷:0鑾峰緱,1娑堣垂',
  `Number` smallint(5) unsigned DEFAULT NULL COMMENT '娓犻亾:0鍏呭�,1娓告垙,2缃戠',
  `serverid` smallint(5) unsigned DEFAULT NULL COMMENT '鏈嶅姟鍣╥d',
  `TargetType` tinyint(4) DEFAULT NULL COMMENT '鍙戠敓鏃堕棿',
  `TargetList` varchar(1024) COLLATE gbk_bin DEFAULT NULL COMMENT '鐗╁搧ID',
  `OperatorID` int(10) unsigned DEFAULT NULL,
  `OperateTime` datetime DEFAULT NULL COMMENT '鐗╁搧鍚嶇О',
  `Reason` varchar(500) COLLATE gbk_bin DEFAULT NULL COMMENT '鏁伴噺',
  `dealTime` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=26 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_keyword` */

DROP TABLE IF EXISTS `t_keyword`;

CREATE TABLE `t_keyword` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `keyword` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=856 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_needloggoods` */

DROP TABLE IF EXISTS `t_needloggoods`;

CREATE TABLE `t_needloggoods` (
  `goodsid` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`goodsid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_onlineinfo` */

DROP TABLE IF EXISTS `t_onlineinfo`;

CREATE TABLE `t_onlineinfo` (
  `serverid` smallint(5) unsigned NOT NULL,
  `onlinenum` int(10) unsigned NOT NULL,
  `time` datetime NOT NULL
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
  PRIMARY KEY (`recordid`),
  KEY `index_userid` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=52805 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_platforminfo` */

DROP TABLE IF EXISTS `t_platforminfo`;

CREATE TABLE `t_platforminfo` (
  `PlatformID` int(10) unsigned NOT NULL,
  `Name` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `Ip` varchar(30) COLLATE gbk_bin DEFAULT NULL,
  `Port` smallint(6) NOT NULL,
  `AppKey` varchar(33) COLLATE gbk_bin DEFAULT NULL,
  `SecretKey` varchar(33) COLLATE gbk_bin DEFAULT NULL,
  `LoginUrl` varchar(256) COLLATE gbk_bin DEFAULT NULL,
  `RegisterUrl` varchar(256) COLLATE gbk_bin DEFAULT NULL,
  `MerchantKey` varchar(33) COLLATE gbk_bin DEFAULT NULL COMMENT '娓告垙鍘傚晢KEY',
  `DesKey` varchar(33) COLLATE gbk_bin DEFAULT NULL COMMENT 'DES_KEY',
  `MerchantID` int(11) DEFAULT NULL COMMENT '鍘傚晢缂栧彿',
  `GameID` int(11) DEFAULT NULL COMMENT '娓告垙缂栧彿',
  `ServerID` int(11) DEFAULT NULL COMMENT '鏈嶅姟鍣ㄧ紪鍙�,
  PRIMARY KEY (`PlatformID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_platforminterface` */

DROP TABLE IF EXISTS `t_platforminterface`;

CREATE TABLE `t_platforminterface` (
  `PlatformID` int(11) NOT NULL,
  `InterfaceID` int(11) DEFAULT NULL,
  `Url` varchar(256) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`PlatformID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_publicnotic` */

DROP TABLE IF EXISTS `t_publicnotic`;

CREATE TABLE `t_publicnotic` (
  `MsgID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '娑堟伅ID',
  `Server` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `BeginTime` datetime DEFAULT NULL COMMENT '寮�鏃堕棿',
  `EndTime` datetime DEFAULT NULL COMMENT '缁撴潫鏃堕棿',
  `MsgContent` varchar(1024) COLLATE gbk_bin DEFAULT NULL COMMENT '鍏憡鍐呭',
  `OperatorID` int(10) unsigned DEFAULT NULL COMMENT '鎿嶄綔鍛�,
  `OperateTime` datetime DEFAULT NULL COMMENT '鎿嶄綔鏃堕棿',
  PRIMARY KEY (`MsgID`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_realno` */

DROP TABLE IF EXISTS `t_realno`;

CREATE TABLE `t_realno` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '璁板綍id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛id',
  `SealNoBegin` datetime DEFAULT NULL COMMENT '灏佸彿寮�鏃堕棿',
  `SealNoEnd` datetime DEFAULT NULL COMMENT '灏佸彿缁撴潫鏃堕棿',
  `sealNoOperatorID` int(10) unsigned DEFAULT NULL COMMENT '灏佸彿鎿嶄綔鍛�,
  `SealNoReason` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '灏佸彿鍘熷洜',
  `SealNoOperateTime` datetime DEFAULT NULL COMMENT '灏佸彿鎿嶄綔鏃堕棿',
  `DeblockingTime` datetime DEFAULT NULL COMMENT '瑙ｅ皝鏃堕棿',
  `DeblockingOperatorID` int(10) unsigned DEFAULT NULL COMMENT '瑙ｅ皝鎿嶄綔鍛�,
  `DeblockingReason` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '瑙ｅ皝鍘熷洜',
  `DeblockingOperateTime` datetime DEFAULT NULL COMMENT '瑙ｅ皝鎿嶄綔鏃堕棿',
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_recharge` */

DROP TABLE IF EXISTS `t_recharge`;

CREATE TABLE `t_recharge` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `orderid` char(50) COLLATE gbk_bin NOT NULL COMMENT '璁㈠崟鍙�,
  `amount` int(11) NOT NULL DEFAULT '0' COMMENT '鏀粯閲戦(鍗曚綅:瑙�',
  `godstone` int(11) NOT NULL,
  `userid` int(11) NOT NULL COMMENT '鐜╁ID',
  `serverid` smallint(6) unsigned NOT NULL COMMENT '鏈嶅姟鍣↖D',
  `rechargetime` datetime NOT NULL COMMENT '鏀粯鏃堕棿',
  `rechargeip` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `chanel` tinyint(4) DEFAULT NULL COMMENT '娓犻亾锛�:瀹樻柟,1:鏂版氮,2:褰撲箰',
  `descript` varchar(2500) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_rechargefaillog` */

DROP TABLE IF EXISTS `t_rechargefaillog`;

CREATE TABLE `t_rechargefaillog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(10) DEFAULT NULL,
  `rechargetime` datetime DEFAULT NULL,
  `amount` int(11) DEFAULT NULL,
  `chanel` tinyint(4) DEFAULT NULL,
  `serverid` smallint(5) DEFAULT NULL,
  `orderid` varchar(50) COLLATE gbk_bin DEFAULT NULL,
  `descript` varchar(2500) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_rechargelog` */

DROP TABLE IF EXISTS `t_rechargelog`;

CREATE TABLE `t_rechargelog` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛',
  `rechargetime` datetime DEFAULT NULL COMMENT '鍏呭�鏃堕棿',
  `amount` int(11) DEFAULT '0' COMMENT '鍏呭�閲戝竵',
  `godstone` int(11) DEFAULT NULL COMMENT '鍏呭�鑾峰緱浠欑煶',
  `chanel` tinyint(4) DEFAULT NULL COMMENT '鍏呭�娓犻亾',
  `serverid` smallint(5) DEFAULT NULL,
  `rechargeip` varchar(20) COLLATE gbk_bin DEFAULT NULL COMMENT '鍏呭�ip',
  `orderid` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '涓庣涓夋柟鍏宠仈浜嬪姟id',
  `descript` varchar(2500) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `index_order` (`orderid`)
) ENGINE=InnoDB AUTO_INCREMENT=27 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_rechargetemplog` */

DROP TABLE IF EXISTS `t_rechargetemplog`;

CREATE TABLE `t_rechargetemplog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `orderid` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '璁㈠崟鍙�,
  `userid` int(10) DEFAULT NULL COMMENT '鐜╁ID',
  `orderuid` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '鐜╁鏂版氮ID',
  `serverid` smallint(4) DEFAULT NULL COMMENT '鏈嶅姟鍣↖D',
  `amount` int(11) DEFAULT NULL COMMENT '鍏呭�閲戦,鍗曚綅锛氳',
  `godstone` int(11) DEFAULT NULL COMMENT '鑾峰緱鐨勪粰鐭�,
  `rechargetime` datetime DEFAULT NULL COMMENT '璁板綍鏃堕棿',
  `chanel` tinyint(4) DEFAULT NULL COMMENT '鍏呭�娓犻亾',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_resource` */

DROP TABLE IF EXISTS `t_resource`;

CREATE TABLE `t_resource` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '璁板綍ID',
  `OperateType` tinyint(3) unsigned DEFAULT NULL COMMENT '鎿嶄綔绫诲瀷:0澧炲姞,1鎵ｉ櫎',
  `xianstone_num` int(8) DEFAULT NULL COMMENT '浠欑煶鏁伴噺',
  `lingstone_num` int(8) DEFAULT NULL COMMENT '鐏电煶鏁伴噺',
  `liquan_num` int(8) DEFAULT NULL COMMENT '绀煎埜鏁伴噺',
  `ServerID` smallint(5) unsigned DEFAULT NULL COMMENT '鏈嶅姟鍣↖D',
  `TargetType` tinyint(3) unsigned DEFAULT NULL COMMENT '鐩爣绫诲瀷:0鎵�湁鐜╁,1鎸囧畾鐜╁',
  `TargetList` varchar(1024) COLLATE gbk_bin DEFAULT NULL COMMENT '鐢ㄦ埛鍒楄〃锛岄�鍙峰垎闅�,
  `OperatorID` int(10) unsigned DEFAULT NULL COMMENT '鎿嶄綔鍛業D',
  `OperateTime` datetime DEFAULT NULL COMMENT '鎿嶄綔鏃堕棿',
  `Reason` varchar(500) COLLATE gbk_bin DEFAULT NULL COMMENT '鎿嶄綔鍘熷洜',
  `MailID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `Resource_mailID` (`MailID`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_resourcelog` */

DROP TABLE IF EXISTS `t_resourcelog`;

CREATE TABLE `t_resourcelog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `OperateType` tinyint(3) unsigned DEFAULT NULL,
  `xianstone_num` int(8) DEFAULT NULL,
  `lingstone_num` int(8) DEFAULT NULL,
  `liquan_num` int(8) DEFAULT NULL,
  `serverid` smallint(5) unsigned DEFAULT NULL,
  `TargetType` tinyint(3) unsigned DEFAULT NULL,
  `TargetList` varchar(1024) COLLATE gbk_bin DEFAULT NULL,
  `OperatorID` int(10) unsigned DEFAULT NULL,
  `OperateTime` datetime DEFAULT NULL,
  `Reason` varchar(500) COLLATE gbk_bin DEFAULT NULL,
  `dealTime` datetime DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_serverinfo` */

DROP TABLE IF EXISTS `t_serverinfo`;

CREATE TABLE `t_serverinfo` (
  `serverid` smallint(5) unsigned NOT NULL AUTO_INCREMENT COMMENT '鏈嶅姟鍣╥d',
  `servername` varchar(18) COLLATE gbk_bin DEFAULT NULL COMMENT '鏈嶅姟鍣ㄥ悕绉�,
  `cooperation` tinyint(3) unsigned DEFAULT NULL COMMENT '娓告垙骞冲彴:0璁父,1鏂版氮,2褰撲箰,3涔濇父,4鑵捐',
  `serverip` varchar(20) COLLATE gbk_bin DEFAULT NULL COMMENT '鏈嶅姟鍣╥p',
  `serverport` smallint(5) unsigned DEFAULT NULL COMMENT '鏈嶅姟鍣ㄧ鍙�,
  `serverstate` tinyint(4) DEFAULT NULL COMMENT '0:鍏抽棴锛�:寮�惎',
  `servertype` tinyint(4) DEFAULT NULL COMMENT '绫诲瀷:0鐧婚檰鏈�1娓告垙鏈�2鑱婂ぉ鏈�,
  `dbName` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `dbip` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `port` smallint(6) DEFAULT NULL,
  `username` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `password` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `ServiceTel` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `ServiceEmail` varchar(50) COLLATE gbk_bin DEFAULT NULL,
  `ServiceQQ` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `FirstOpenServiceTime` datetime DEFAULT NULL COMMENT '绗竴娆″紑鏈嶆椂闂�,
  PRIMARY KEY (`serverid`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_serverlog` */

DROP TABLE IF EXISTS `t_serverlog`;

CREATE TABLE `t_serverlog` (
  `ServerID` smallint(5) unsigned NOT NULL COMMENT '鏈嶅姟鍣↖D',
  `type` tinyint(4) DEFAULT NULL COMMENT '鎿嶄綔绫诲瀷:0鏈嶅姟鍚姩锛�鏈嶅姟鍏抽棴,2浠欑煶锛�鐏电煶锛�鐗╁搧锛�閭欢锛�娑堟伅',
  `FinishTime` datetime DEFAULT NULL COMMENT '瀹屾垚鏃堕棿',
  `RecordID` int(10) unsigned DEFAULT NULL COMMENT '瀵瑰簲鐨勮褰旾D'
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_spiritstonelog` */

DROP TABLE IF EXISTS `t_spiritstonelog`;

CREATE TABLE `t_spiritstonelog` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
  `userid` int(10) unsigned DEFAULT NULL COMMENT '鐢ㄦ埛id',
  `type` tinyint(3) unsigned DEFAULT NULL COMMENT '绫诲瀷:0鑾峰緱,1娑堣垂',
  `chanel` tinyint(4) DEFAULT NULL COMMENT '娓犻亾:0鍏呭�,1娓告垙,2缃戠',
  `serverid` smallint(6) DEFAULT NULL COMMENT '鏈嶅姟鍣╥d',
  `happentime` datetime DEFAULT NULL COMMENT '鍙戠敓鏃堕棿',
  `varspiritstone` int(11) DEFAULT NULL COMMENT '鍙樺姩浠欑煶',
  `afterspiritstone` int(11) DEFAULT NULL COMMENT '鍙樺姩鍚庝粰鐭�,
  `description` varchar(100) COLLATE gbk_bin DEFAULT NULL COMMENT '璇存槑',
  `associateid` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '鍏宠仈ID',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_sysmail` */

DROP TABLE IF EXISTS `t_sysmail`;

CREATE TABLE `t_sysmail` (
  `MailID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '閭欢ID',
  `ServerID` smallint(5) unsigned DEFAULT NULL COMMENT '鏈嶅姟鍣↖D,闆惰〃绀哄叏鏈�,
  `TargetType` tinyint(4) DEFAULT NULL COMMENT '鐩爣绫诲瀷锛�鎵�湁鐜╁锛�鎸囧畾鐜╁锛岋紥鏂版敞鍐岀帺瀹�,
  `TargetList` varchar(1024) COLLATE gbk_bin DEFAULT NULL COMMENT '鐩爣鍒楄〃锛岀敤閫楀彿鍒嗛殧',
  `Title` varchar(50) COLLATE gbk_bin DEFAULT NULL COMMENT '閭欢鏍囬',
  `Content` varchar(300) COLLATE gbk_bin DEFAULT NULL COMMENT '姝ｆ枃',
  `OperatorID` int(10) unsigned DEFAULT NULL COMMENT '鎿嶄綔鍛�,
  `OperateTime` datetime DEFAULT NULL COMMENT '鎿嶄綔鏃堕棿',
  `sendtime` datetime DEFAULT NULL,
  PRIMARY KEY (`MailID`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_sysmsg` */

DROP TABLE IF EXISTS `t_sysmsg`;

CREATE TABLE `t_sysmsg` (
  `MsgID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '娑堟伅ID',
  `MsgType` tinyint(4) DEFAULT '0' COMMENT '绫诲瀷:0鑱婂ぉ妗�1涓荤晫闈�,
  `BeginTime` datetime DEFAULT NULL COMMENT '寮�鏃堕棿',
  `EndTime` datetime DEFAULT NULL COMMENT '缁撴潫鏃堕棿',
  `IntervalTime` int(10) unsigned DEFAULT '0' COMMENT '鍙戦�闂撮殧鏃堕棿锛屽崟浣嶏細绉�鑱婂ぉ妗嗙敤鍒�',
  `ServerID` smallint(5) unsigned DEFAULT NULL COMMENT '鍙戝箍鎾秷鎭殑鏈嶅姟鍣紝0琛ㄧず鍏ㄦ湇',
  `MsgContent` varchar(500) COLLATE gbk_bin DEFAULT NULL COMMENT '娑堟伅鍐呭',
  `OperatorID` int(10) unsigned DEFAULT NULL COMMENT '鎿嶄綔鍛�,
  `operateTime` datetime DEFAULT NULL COMMENT '鎿嶄綔鏃堕棿',
  `state` tinyint(4) DEFAULT NULL COMMENT '1:鍋滅敤 0:鍚敤',
  `dealTime` datetime DEFAULT NULL,
  PRIMARY KEY (`MsgID`)
) ENGINE=InnoDB AUTO_INCREMENT=57 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

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
) ENGINE=InnoDB AUTO_INCREMENT=2507095 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_updatedesc` */

DROP TABLE IF EXISTS `t_updatedesc`;

CREATE TABLE `t_updatedesc` (
  `MsgID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '娑堟伅ID',
  `BeginTime` datetime DEFAULT NULL COMMENT '寮�鏃堕棿',
  `EndTime` datetime DEFAULT NULL COMMENT '缁撴潫鏃堕棿',
  `MsgContent` varchar(1024) COLLATE gbk_bin DEFAULT NULL COMMENT '鍏憡鍐呭',
  `OperatorID` int(10) unsigned DEFAULT NULL COMMENT '鎿嶄綔鍛�,
  `OperateTime` datetime DEFAULT NULL COMMENT '鎿嶄綔鏃堕棿',
  PRIMARY KEY (`MsgID`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_user` */

DROP TABLE IF EXISTS `t_user`;

CREATE TABLE `t_user` (
  `UserID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '鐢ㄦ埛ID',
  `UserName` varchar(81) COLLATE gbk_bin NOT NULL,
  `Password` char(66) COLLATE gbk_bin DEFAULT NULL COMMENT '瀵嗙爜',
  `Secret` char(35) COLLATE gbk_bin DEFAULT NULL,
  `channel` tinyint(4) DEFAULT NULL COMMENT '娓犻亾:0璁父,1鏂版氮,2褰撲箰,3涔濇父,4鑵捐',
  `registertime` datetime DEFAULT NULL COMMENT '娉ㄥ唽鏃堕棿',
  `registerip` varchar(20) COLLATE gbk_bin DEFAULT 'NULL' COMMENT '娉ㄥ唽ip',
  `IdentityStatus` tinyint(4) DEFAULT '0' COMMENT '娌夎糠璁よ瘉鐘舵�:0鏈璇�1灏忎簬18宀侊紝2澶т簬绛変簬18宀�,
  `SealNoID` int(10) unsigned DEFAULT '0' COMMENT '灏佸彿璁板綍id',
  `onlinetime` int(11) DEFAULT '0' COMMENT '鍦ㄧ嚎鏃堕暱',
  `lastlogintime` datetime DEFAULT NULL COMMENT '鏈�悗鐧婚檰鏃堕棿',
  `lastloginip` varchar(20) COLLATE gbk_bin DEFAULT 'NULL' COMMENT '鏈�悗鐧婚檰ip',
  `onlineserverid` smallint(6) DEFAULT '0' COMMENT '鍦ㄧ嚎鏈嶅姟鍣�涓�琛ㄧず涓嶅湪绾�,
  `onlinerecordid` int(10) unsigned DEFAULT '0' COMMENT '鍦ㄧ嚎璁板綍ID',
  PRIMARY KEY (`UserID`),
  KEY `index_username` (`UserName`)
) ENGINE=InnoDB AUTO_INCREMENT=55280 DEFAULT CHARSET=gbk COLLATE=gbk_bin CHECKSUM=1 DELAY_KEY_WRITE=1 ROW_FORMAT=DYNAMIC;

/*Table structure for table `t_useractor` */

DROP TABLE IF EXISTS `t_useractor`;

CREATE TABLE `t_useractor` (
  `userid` int(10) unsigned NOT NULL,
  `serverid` smallint(5) unsigned NOT NULL,
  `actorname` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`userid`,`serverid`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `t_version` */

DROP TABLE IF EXISTS `t_version`;

CREATE TABLE `t_version` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `LastVersion` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `MinVersion` varchar(20) COLLATE gbk_bin DEFAULT NULL,
  `OperatorID` int(10) unsigned DEFAULT NULL,
  `OperateTime` datetime DEFAULT NULL,
  `InstallPackageUrl` varchar(300) COLLATE gbk_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gbk COLLATE=gbk_bin;

/*Table structure for table `user` */

DROP TABLE IF EXISTS `user`;

CREATE TABLE `user` (
  `user_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(250) NOT NULL,
  `user_password` varchar(250) NOT NULL,
  `user_regip` varchar(50) NOT NULL,
  `user_lastip` varchar(50) DEFAULT NULL,
  `user_regtime` datetime NOT NULL,
  `user_lasttime` datetime DEFAULT NULL,
  `user_state` tinyint(4) NOT NULL DEFAULT '0' COMMENT '0锛氭湭绂佹锛�锛氱姝�,
  `user_group` int(11) NOT NULL,
  `xianstone_num` int(11) NOT NULL DEFAULT '0',
  `lingstone_num` int(11) NOT NULL DEFAULT '0',
  `liquan_num` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`)
) ENGINE=MyISAM AUTO_INCREMENT=48 DEFAULT CHARSET=utf8 COMMENT='鍚庡彴瀹㈡湇琛�;

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

/* Procedure structure for procedure `P_CDB_AddAllPermissionGMCmd` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_AddAllPermissionGMCmd` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_AddAllPermissionGMCmd`(pLoginName char(18))
BEGIN
	declare t_userID int unsigned default 0;
	declare t_userID2 int unsigned default 0;
	select UserID into t_userID from t_user where UserName = pLoginName;
	select UserID into t_userID2 from t_controlcmd where UserID = t_userID;
	if t_userID2 = 0 then
		insert into t_controlcmd values(t_userID, "-1,");
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_BackSysMail` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_BackSysMail` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_BackSysMail`(pID int unsigned , pServerID smallint unsigned)
BEGIN
	declare t_id int unsigned default 0;
	
	select MailID into t_id from t_sysmail where MailID = pID and (ServerID = pServerID or ServerID = 0);
	if t_id = 0 then
		select 2;
		select 0,0,"","","";
	else
		select 0;
		select MailID,TargetType,TargetList,Title,Content from t_sysmail where MailID = pID and (ServerID = pServerID or ServerID = 0);
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_ClearServerData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_ClearServerData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_ClearServerData`(pServerID smallint unsigned)
BEGIN
	delete from t_useractor where serverid = pServerID;
	delete from t_onlinelog where onlineserverid = pServerID;
	update t_serverinfo set FirstOpenServiceTime = 0 where serverid = pServerID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_DontTalk` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_DontTalk` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_DontTalk`(pID int unsigned)
BEGIN
	declare t_id int unsigned default 0;
	declare t_userid int unsigned default 0;
	declare t_BeginTime datetime default 0;
	declare t_EndTime datetime default 0;
	declare t_RemoveTime datetime default 0;
	declare t_UnixBeginTime int unsigned default 0;
	declare t_UnixEndTime int unsigned default 0;
	declare t_UnixRemoveTime int unsigned default 0;
	select id,userid,donttalkbegin,donttalkend,removetime into t_id,t_userid,t_BeginTime,t_EndTime,t_RemoveTime from t_donttalk where id = pID;
	set t_UnixBeginTime = UNIX_TIMESTAMP(t_BeginTime);
	set t_UnixEndTime = UNIX_TIMESTAMP(t_EndTime);
	set t_UnixRemoveTime = UNIX_TIMESTAMP(t_RemoveTime);
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0;
	else
		set t_UnixBeginTime = UNIX_TIMESTAMP(t_BeginTime);
		set t_UnixEndTime = UNIX_TIMESTAMP(t_EndTime);
		set t_UnixRemoveTime = UNIX_TIMESTAMP(t_RemoveTime);
		select 0;
		select t_id,t_userid,t_UnixBeginTime,t_UnixEndTime,t_UnixRemoveTime;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetAllDontTalk` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetAllDontTalk` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_GetAllDontTalk`(pServerID smallint unsigned)
BEGIN
	declare t_id int unsigned default 0;
	select id into t_id from t_donttalk limit 0,1;
	
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0;
	else
		select 0;
		select id,userid,UNIX_TIMESTAMP(donttalkbegin),UNIX_TIMESTAMP(donttalkend),UNIX_TIMESTAMP(removetime) from t_donttalk
			where serverid=pServerID and donttalkend > now() and (removetime > now() || removetime is NULL);
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetAllSealNo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetAllSealNo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_GetAllSealNo`(pServerID smallint unsigned)
BEGIN
	declare t_id int unsigned default 0;
	select id into t_id from t_realno limit 0, 1;
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0;
	else
		select 0;
		select id,userid,UNIX_TIMESTAMP(SealNoBegin),UNIX_TIMESTAMP(SealNoEnd),UNIX_TIMESTAMP(DeblockingTime) from t_realno where SealNoEnd > now() and (DeblockingTime > now() || DeblockingTime is NULL);
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetKeyWordInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetKeyWordInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetKeyWordInfo`()
BEGIN
          select keyword from t_keyword;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetPlatformInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetPlatformInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetPlatformInfo`(pPlatformID smallint unsigned)
BEGIN
          declare tThridPartIp varchar(30)  default "";
          declare tThridPartPort smallint  default 0;
          declare tThridPartAppKey varchar(33)  default "";
          declare tThridPartSecretKey varchar(33)  default "";
          declare tMerchantKey varchar(33)  default "";
          declare tDesKey varchar(33)  default "";
          declare tMerchantID int default 0;
          declare tGameID int default 0;
          declare tServerID int default 0;
 
          declare tInterfaceID int default 0;
          select Ip,Port,AppKey,SecretKey,MerchantKey,DesKey,MerchantID,GameID,ServerID 
              into tThridPartIp,tThridPartPort,tThridPartAppKey,tThridPartSecretKey,tMerchantKey,tDesKey,tMerchantID,tGameID,tServerID from t_platforminfo where PlatformID=pPlatformID;
          select tThridPartIp,tThridPartPort,tThridPartAppKey,tThridPartSecretKey,tMerchantKey,tDesKey,tMerchantID,tGameID,tServerID;
          select InterfaceID into tInterfaceID from t_platforminterface where PlatformID= pPlatformID limit 0,1;
          if tInterfaceID = 0 then
                select 0,"";
          else
                select InterfaceID ,Url from t_platforminterface where PlatformID= pPlatformID;
          end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetPublicNotic` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetPublicNotic` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetPublicNotic`()
BEGIN
          declare tPublicNotic varchar(1024) default "";
          declare tUpdateDesc varchar(1024) default "";
          declare tLastVersion    varchar(20)   default "";
          declare tMinVersion  varchar(20)  default "";
          declare tInstallPackageUrl varchar(300) default "";
          select MsgContent into tPublicNotic from t_publicnotic where now() >= BeginTime and now() <= EndTime order by MsgID desc limit 0,1;
          select MsgContent into tUpdateDesc from t_UpdateDesc where now() >= BeginTime and now() <= EndTime order by MsgID desc limit 0,1;
           select LastVersion,MinVersion ,InstallPackageUrl into tLastVersion,tMinVersion,tInstallPackageUrl from t_version  order by id desc limit 0,1;
          select tPublicNotic,tUpdateDesc,tLastVersion,tMinVersion,tInstallPackageUrl;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetSealNo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetSealNo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_GetSealNo`(pID int unsigned)
BEGIN
	declare t_id int unsigned default 0;
	declare t_userid int unsigned default 0;
	declare t_BeginTime datetime default 0;
	declare t_EndTime datetime default 0;
	declare t_RemoveTime datetime default 0;
	declare t_UnixBeginTime int unsigned default 0;
	declare t_UnixEndTime int unsigned default 0;
	declare t_UnixRemoveTime int unsigned default 0;
	
	select id,userid,SealNoBegin,SealNoEnd,DeblockingTime into t_id,t_userid,t_BeginTime,t_EndTime,t_RemoveTime from t_realno where id = pID;
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0;
	else
		set t_UnixBeginTime = UNIX_TIMESTAMP(t_BeginTime);
		set t_UnixEndTime = UNIX_TIMESTAMP(t_EndTime);
		set t_UnixRemoveTime = UNIX_TIMESTAMP(t_RemoveTime);
		select 0;
		select t_id,t_userid,t_UnixBeginTime,t_UnixEndTime,t_UnixRemoveTime;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetServerInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetServerInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetServerInfo`(pServerID smallint unsigned,pPlatformID smallint unsigned)
BEGIN
          declare tPublicNotic varchar(1024) default "";
          declare tUpdateDesc varchar(1024) default "";
          declare tLastVersion    varchar(20)   default "";
          declare tMinVersion  varchar(20)  default "";
          declare tInstallPackageUrl varchar(300) default "";
          declare tServiceTel varchar(20) default "";
          declare tServiceEmail varchar(50) default "";
          declare tServiceQQ  varchar(20)   default "";
          declare tThridPartIp varchar(30)  default "";
          declare tThridPartPort smallint  default 0;
          declare tThridPartAppKey varchar(33)  default "";
          declare tThridPartSecretKey varchar(33)  default "";
          declare tThridPartLoginUrl varchar(256)  default "";
          declare tThridPartRegisterUrl varchar(256)  default "";
          
          select MsgContent into tPublicNotic from t_publicnotic where now() <= EndTime order by MsgID desc limit 0,1;
          select MsgContent into tUpdateDesc from t_UpdateDesc where  now() <= EndTime order by MsgID desc limit 0,1;
          select LastVersion,MinVersion ,InstallPackageUrl into tLastVersion,tMinVersion,tInstallPackageUrl from t_version  order by id desc limit 0,1;
          select Ip,Port,AppKey,SecretKey,LoginUrl,RegisterUrl 
            into tThridPartIp,tThridPartPort,tThridPartAppKey,tThridPartSecretKey,tThridPartLoginUrl,tThridPartRegisterUrl  from t_platforminfo where PlatformID=pPlatformID;
           select ServiceTel,ServiceEmail,ServiceQQ into tServiceTel,tServiceEmail,tServiceQQ from t_serverinfo where serverid=pServerID;
	   if tServiceEmail = "" then
		select ServiceEmail into tServiceEmail from t_serverinfo where servertype = 0;
	   end if;
          select tPublicNotic,tUpdateDesc,tLastVersion,tMinVersion,tInstallPackageUrl,tServiceTel,tServiceEmail,tServiceQQ
          ,tThridPartIp,tThridPartPort,tThridPartAppKey,tThridPartSecretKey,tThridPartLoginUrl,tThridPartRegisterUrl;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetUserInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetUserInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetUserInfo`(pusername varchar(81),pPwd char(66),pIp varchar(33),pChenal tinyint,pSecret varchar(35))
BEGIN
          declare tuserid  int unsigned default 0;
          declare tpassword char(66);
          declare tSealNoID int unsigned default 0;  -- 灏佸彿
          declare pRetCode int;
          declare tuserid2  int unsigned default 0;
          declare tSealNoEnd datetime;
          declare tDeblockingTime datetime;
          declare tIdentityStatus tinyint default 0;
          declare tOnlineTime int default 0;
	  declare tAllow int unsigned default 0;
       
          select UserID,Password,SealNoID,IdentityStatus into tuserid,tpassword,tSealNoID,tIdentityStatus from t_user where UserName = pusername and pChenal=channel;
	  select UserID into tAllow from t_allowenter where UserID = tuserid or UserID = 4294967295 limit 0,1;
        
           if tuserid = 0 and pChenal <> 0 then
               insert into t_user(UserName,Password,Secret,channel,registertime,registerip) values(pusername,pPwd,pSecret,pChenal,now(),pIp);
               select UserID,Password,SealNoID,IdentityStatus into tuserid,tpassword,tSealNoID,tIdentityStatus from t_user where UserName = pusername and pChenal=channel;
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
            set pRetCode = 1;  -- 閹存劕濮�
           if tSealNoID <> 0 then
                select userid,SealNoEnd,DeblockingTime into tuserid2,tSealNoEnd, tDeblockingTime from t_realno where id = tSealNoID;
                if tuserid2 <> tuserid or now() > tSealNoEnd or (tDeblockingTime is not null and now() > tDeblockingTime) then
                      update t_user set SealNoID = 0 where UserID = tuserid;
                else
                   set pRetCode = -5;  -- 琚皝鍙�
                end if;
           end if;
            if pRetCode = 1 then                
              update t_user set lastlogintime = now(), lastloginip = pIp  where UserID = tuserid ;   
            end if;
   
        end if;
         -- 鏈�繎浜斿皬鏃跺湪绾挎椂闀�
         if tIdentityStatus <> 2 and pRetCode = 1 then
           select sum(unix_timestamp(logoutime)-unix_timestamp(logintime)) into tOnlineTime from t_onlinelog where userid = tuserid and logoutime is not null and unix_timestamp(now()) - unix_timestamp(logoutime) < 5*3600;
         end if;
        
          select pRetCode ,tuserid,tIdentityStatus,tOnlineTime;        
          select serverid from t_useractor where userid = tuserid;
  END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetUserInfoTest` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetUserInfoTest` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_GetUserInfoTest`(pusername blob)
BEGIN
          insert into t_test(Username)values(pusername);
          select 0;
          select 0;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GetUserPermission` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GetUserPermission` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_GetUserPermission`(pUserID int unsigned)
BEGIN
	declare t_userid int unsigned default 0;
	select UserID into t_userid from t_controlcmd where UserID = pUserID;
	if t_userid = 0 then
		select 2;
		select "";
	else
		select 0;
		select Permission from t_controlcmd where UserID = pUserID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_AllPayData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_AllPayData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_AllPayData`(pUserID int unsigned,pServerID smallint unsigned)
BEGIN
	declare t_id int unsigned default 0;
	select id into t_id from t_recharge where userID=pUserID and serverid=pServerID;
	
	if t_id = 0 then
		select 2;
		select 0,0,0,0;
	else
		select 0;
		select id,amount,godstone,userid from t_recharge where userid=pUserID and serverid=pServerID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_FirstOpenServerTime` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_FirstOpenServerTime` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_FirstOpenServerTime`(pServerID smallint unsigned, pPlatformID smallint unsigned)
BEGIN
	declare t_firstOpenServiceTime int unsigned default 0;
	declare t_serverid smallint unsigned default 0;
	select serverid,UNIX_TIMESTAMP(FirstOpenServiceTime) into t_serverid,t_firstOpenServiceTime from t_serverinfo where serverid = pServerID;
	
	if t_serverid = 0 then
		select 2;
		select 0;
	else
		select 0;
		select t_firstOpenServiceTime;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_NeedLogGoodsID` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_NeedLogGoodsID` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_NeedLogGoodsID`()
BEGIN
	declare t_goodsid smallint unsigned default 0;
	select goodsid into t_goodsid from t_needloggoods limit 0,1;
	if t_goodsid = 0 then
		select 2;
		select 0;
	else
		select 0;
		select goodsid from t_needloggoods;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_PayData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_PayData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_PayData`(pID int unsigned, pServerID smallint unsigned)
BEGIN
	declare t_id int unsigned default 0;
	declare t_money int default 0;
	declare t_godstone int default 0;
	declare t_userID int default 0;
	declare t_orderid char(50) default "";
	declare t_logid int unsigned default 0;
	
	select id,amount,godstone,userid,orderid into t_id,t_money,t_godstone,t_userID,t_orderid from t_recharge where id = pID and serverid = pServerID;
	select id into t_logid from t_rechargelog where orderid = t_orderid;
	
	if t_logid = 0 then
		
		if t_id = 0 then
			select 2;
			select 0,0,0,0;
		else
			select 0;
			select t_id,t_money,t_godstone,t_userID;
		end if;
	else
		delete from t_recharge where id = pID;
		select 2;
		select 0,0,0,0;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_ServiceData` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_ServiceData` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_ServiceData`(pServerID int unsigned)
BEGIN
	declare t_serverid int unsigned default 0;
	declare t_ServiceTel varchar(20) default "";
	declare t_ServiceEmail varchar(50) default "";
	declare t_ServiceQQ varchar(20) default "";
	select serverid,ServiceTel,ServiceEmail,ServiceQQ into t_serverid,t_ServiceTel,t_ServiceEmail,t_ServiceQQ from t_serverinfo where serverid = pServerID;
	if t_serverid = 0 then
		select 2;
		select "","","";
	else
		if t_ServiceEmail = "" then
			select ServiceEmail into t_ServiceEmail from t_serverinfo where servertype = 0;
		end if;
		select 0;
		select t_ServiceTel,t_ServiceEmail,t_ServiceQQ;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Get_Version` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Get_Version` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Get_Version`(pId int unsigned)
BEGIN
	declare t_id int unsigned default 0;
	declare t_LastVersion varchar(20) default "";
	declare t_MinVersion varchar(20) default "";
	declare t_InstallPackageUrl varchar(20) default "";
	select id,LastVersion,MinVersion,InstallPackageUrl into t_id,t_LastVersion,t_MinVersion,t_InstallPackageUrl from t_version where id=pId;
	if t_id = 0 then
		select 2;
		select 0,0,0,0;
	else
		select 0;
		select t_id,t_LastVersion,t_MinVersion,t_InstallPackageUrl;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GiveGoods` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GiveGoods` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_GiveGoods`(pMailID int unsigned, pServerID smallint unsigned)
BEGIN
	declare t_mailID int unsigned default 0;
	declare t_id int unsigned default 0;
	
	select MailID into t_mailID from t_sysmail where MailID = pMailID and ServerID = pServerID;
	if t_mailID = 0 then
		select 2,0,0,"","","";
		select 0,0,0,0;
	else
		select id into t_id from t_goods where MailID = pMailID and ServerID = pServerID limit 0,1;
		if t_id = 0 then
			select 2,0,0,"","","";
			select 0,0,0,0;
		else
			select 0,MailID,TargetType,TargetList,Title,Content from t_sysmail where MailID = pMailID and ServerID = pServerID;
			select id,OperateType,GoodsID,Number from t_goods where MailID = pMailID and ServerID = pServerID;
			insert into t_goodslog(OperateType,GoodsID,GoodsName,Number,serverid,TargetType,TargetList,OperatorID,OperateTime,Reason,dealTime)
				select OperateType,GoodsID,GoodsName,Number,ServerID,TargetType,TargetList,OperatorID,OperateTime,Reason, now() from t_goods where MailID = pMailID and ServerID = pServerID;
			delete from t_goods where MailID = pMailID and ServerID = pServerID;
		end if;
	end if;	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_GiveResource` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_GiveResource` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_GiveResource`(pMailID int unsigned, pServerID smallint unsigned)
BEGIN
	declare t_mailID int unsigned default 0;
	declare t_id int unsigned default 0;
	select MailID into t_mailID from t_sysmail where MailID = pMailID and ServerID = pServerID;
	if t_mailID = 0 then
		select 2,0,0,"","","";
		select 0,0,0,0,0;
	else
		select id into t_id from t_resource where MailID = pMailID and ServerID = pServerID;
		
		if t_id = 0 then
			select 2,0,0,"","","";
			select 0,0,0,0,0;
		else
			select 0,MailID,TargetType,TargetList,Title,Content from t_sysmail where MailID = pMailID and ServerID = pServerID;
			select id,OperateType,xianstone_num,lingstone_num,liquan_num from t_resource where MailID = pMailID and ServerID = pServerID;
			insert into t_resourcelog(OperateType,xianstone_num,lingstone_num,liquan_num,serverid,TargetType,TargetList,OperatorID,OperateTime,Reason,dealTime)
				select OperateType,xianstone_num,lingstone_num,liquan_num,ServerID,TargetType,TargetList,OperatorID,OperateTime,Reason,now() from  t_resource where MailID = pMailID and ServerID = pServerID;
			delete from t_resource where MailID = pMailID and ServerID = pServerID; 
		end if;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_InsertUserInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_InsertUserInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_InsertUserInfo`(pusername varchar(81),pPwd char(66),pIp varchar(32),pChanel tinyint unsigned)
BEGIN
           declare tuserid  int unsigned default 0;
          declare tpassword char(66);
          declare pRetCode int;
       
        select UserID into tuserid from t_user where UserName = pusername ;
        
        if tuserid <> 0 then
              set pRetCode = -4;  -- 閻劍鍩涘鎻掔摠閸�         
        else
           -- 閹绘帒鍙嗛弫鐗堝祦
           insert into t_user(UserName,Password,channel,registertime,registerip,lastlogintime,lastloginip) 
                values(pusername,pPwd,pChanel,now(),pIp,now(),pIp);
           set tuserid = LAST_INSERT_ID();
           if tuserid = 0 then
              set pRetCode = -1;
           else
              set pRetCode = 1;  -- 閹存劕濮�      
           end if;       
        end if;
         select pRetCode,tuserid,0,0;
         select 0;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_RemoveToPayLog` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_RemoveToPayLog` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_RemoveToPayLog`(pID int unsigned)
BEGIN
	declare t_id int unsigned default 0;
	declare t_orderid char(50) default "";
	declare t_amount int default 0;
	declare t_getgodstone int default 0;
	declare t_userID int unsigned default "";
	declare t_serverID smallint default 0;
	declare t_time datetime default "";
	declare t_payip varchar(20) default "";
	declare t_chanel tinyint default 0;
	declare t_desc varchar(2500) default "";
	select id,orderid,amount,godstone,userid,serverid,rechargetime,rechargeip,chanel,descript into t_id,t_orderid,t_amount,t_getgodstone,t_userID,t_serverID,t_time,t_payip,t_chanel,t_desc from t_recharge where id = pID;
	if t_id <> 0 then
		delete from t_recharge where id = pID;
		insert into t_rechargelog(userid,rechargetime,amount,godstone,chanel,serverid,rechargeip,orderid,descript) values(t_userID,t_time,t_amount,t_getgodstone,t_chanel,t_serverID,t_payip,t_orderid,t_desc);
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Save_FirstOpenServerTime` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Save_FirstOpenServerTime` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Save_FirstOpenServerTime`(pServerID smallint unsigned, pPlatformID smallint unsigned,pTime int unsigned)
BEGIN
	update t_serverinfo set FirstOpenServiceTime = FROM_UNIXTIME(pTime) where serverid = pServerID; -- and cooperation = pPlatformID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Save_GameGoodsLog` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Save_GameGoodsLog` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Save_GameGoodsLog`(pUserID int unsigned,pType tinyint unsigned,pChanel tinyint unsigned,pGoodsID smallint unsigned,pGoodsUID bigint unsigned,pGoodsNum smallint unsigned,pDescription varchar(300))
BEGIN
	insert into t_gamegoodslog(useid,type,chanel,goodsid,goodsuid,goodsnum,time,descript) values(pUserID,pType,pChanel,pGoodsID,pGoodsUID,pGoodsNum,now(),pDescription);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Save_GodStoneLog` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Save_GodStoneLog` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Save_GodStoneLog`(pUserID int unsigned,pType tinyint unsigned,pChanel tinyint unsigned,pServerID smallint unsigned,pVargodstone int,pAftergodstone int,pDescription varchar(300))
BEGIN
	insert into t_godstonelog(userid,type,chanel,serverid,happentime,vargodstone,aftergodstone,description) values(pUserID,pType,pChanel,pServerID,now(),pVargodstone,pAftergodstone,pDescription);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_Save_OnlineInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_Save_OnlineInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_CDB_Save_OnlineInfo`(pServerID smallint unsigned, pOnlineNum int unsigned)
BEGIN
	insert into t_onlineinfo values(pServerID, pOnlineNum, now());
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_ServerStart` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_ServerStart` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_ServerStart`(pServerID smallint unsigned)
BEGIN
            declare tServerName varchar(20) default "";
            declare tServiceTel varchar(20) default "";
            declare tServiceEmail varchar(50) default "";
            declare tServiceQQ  varchar(20)   default "";
            declare tLastVersion varchar(20)  default "";
            declare tMinVersion  varchar(20)  default "";
            declare tInstallPackageUrl  varchar(300)  default "";
	    declare tFirstOpenServerTime int unsigned default 0;
            update t_user set onlineserverid = 0 where onlineserverid = pServerID;
           
            insert into t_serverlog (ServerID,type,FinishTime,RecordID) values (pServerID,0,now(),0);
           select LastVersion,MinVersion ,InstallPackageUrl into tLastVersion,tMinVersion,tInstallPackageUrl from t_version  order by id desc limit 0,1;
           select servername,ServiceTel,ServiceEmail,ServiceQQ,UNIX_TIMESTAMP(FirstOpenServiceTime) into tServerName,tServiceTel,tServiceEmail,tServiceQQ,tFirstOpenServerTime from t_serverinfo where serverid=pServerID;
	   if tServiceEmail = "" then
		select ServiceEmail into tServiceEmail from t_serverinfo where servertype = 0;
	   end if;
            update t_serverinfo set serverstate = 1 where serverid=pServerID;
           select tLastVersion,tMinVersion,tInstallPackageUrl,tServerName,tServiceTel,tServiceEmail,tServiceQQ,tFirstOpenServerTime;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_ServerStop` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_ServerStop` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_ServerStop`(pServerID smallint unsigned)
BEGIN
            update t_user set onlineserverid = 0 where onlineserverid = pServerID;
            insert into t_serverlog (ServerID,type,FinishTime,RecordID) values (pServerID,1,now(),0);
            update t_serverinfo set serverstate = 0 where serverid=pServerID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_UserEnterGame` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_UserEnterGame` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_UserEnterGame`(pServerID smallint unsigned,pUserID int unsigned,pGolStone int,pIp varchar(32),pFirstEnter tinyint,pActorName varchar(20))
BEGIN
           declare tonlinerecordid int unsigned default 0;
           declare tonlinetime int default 0; -- 鍦ㄧ嚎鏃堕棿
           select onlinerecordid into tonlinerecordid from t_user where userid=pUserID and onlineserverid = pServerID;
           if tonlinerecordid <> 0 then
              select UNIX_TIMESTAMP(now()) - UNIX_TIMESTAMP(logintime)  into tonlinetime from t_onlinelog where recordid = tonlinerecordid for update;
              update t_onlinelog set logoutime = now(),logoutgodstone = logingodstone where recordid = tonlinerecordid;
        
           end if;
           insert into t_onlinelog(userid ,onlineserverid,logintime,loginip,logingodstone) values(pUserID,pServerID,now(),pIp,pGolStone);
           set tonlinerecordid =  LAST_INSERT_ID();
           update t_user set onlineserverid = pServerID,onlinerecordid=tonlinerecordid,onlinetime = onlinetime+tonlinetime where userid=pUserID;
           if pFirstEnter <> 0 then
               insert into t_useractor(userid,serverid,actorname) values(pUserID,pServerID,pActorName);
           end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_CDB_UserExitGame` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_CDB_UserExitGame` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_CDB_UserExitGame`(pServerID smallint unsigned,pUserID int unsigned,pGolStone int)
BEGIN
          declare tonlinerecordid int unsigned default 0;
           declare tonlinetime int default 0; -- 鍦ㄧ嚎鏃堕棿
           select onlinerecordid into tonlinerecordid from t_user where userid=pUserID and onlineserverid = pServerID;
           if tonlinerecordid <> 0 then
              select UNIX_TIMESTAMP(now()) - UNIX_TIMESTAMP(logintime) into tonlinetime from t_onlinelog where recordid = tonlinerecordid for update;
              update t_onlinelog set logoutime = now(),logoutgodstone = pGolStone where recordid = tonlinerecordid;
        
           end if;
           update t_user set onlineserverid = 0,onlinerecordid=0,onlinetime = onlinetime+tonlinetime where userid=pUserID;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Add_ServerInfo` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Add_ServerInfo` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_GDB_Add_ServerInfo`()
BEGIN
	-- 鏈嶅姟鍣ㄥ悕
	declare t_servername varchar(18) default 'jxcq';
	-- 鏈嶅姟鍣ㄦ笭閬�0璁父,1鏂版氮,2褰撲箰,3涔濇父,4鑵捐
	declare t_cooperation tinyint unsigned default 0;
	-- 鏈嶅姟鍣↖P
	declare t_serverip varchar(20) default '127.0.0.1';
	-- 鏈嶅姟鍣ㄧ鍙�
	declare t_serverport smallint unsigned default 5003;
	-- 鏈嶅姟鍣ㄧ姸鎬�
	declare t_serverstate tinyint default 1;
	-- 鏈嶅姟鍣ㄧ被鍨�   0鐧婚檰鏈�1娓告垙鏈�2鑱婂ぉ鏈�
	declare t_servertype tinyint default 1;
	-- 鏁版嵁搴撳悕瀛�
	declare t_dbName varchar(20) default 'gamedb';
	-- 鏁版嵁搴揑P
	declare t_dbip varchar(20) default '127.0.0.1';
	-- 鏁版嵁搴撶鍙�
	declare t_dbport smallint default 3306;
	-- 鏁版嵁搴撶敤鎴峰悕
	declare t_username varchar(20) default 'root';
	-- 鏁版嵁搴撳瘑鐮�
	declare t_password varchar(20) default '888888';
	-- 瀹㈡湇鐢佃瘽
	declare t_servicetel varchar(20) default '12345689';
	-- 瀹㈡湇閭
	declare t_serviceemail varchar(50) default 'jxcqgm@xygames.cn';
	-- 瀹㈡湇QQ
	declare t_serviceqq varchar(20) default '123456';
	insert into t_serverinfo(servername,cooperation,serverip,serverport,serverstate,servertype,dbName,dbip,port,username,password,ServiceTel,ServiceEmail,ServiceQQ)
		values
(t_servername,t_cooperation,t_serverip,t_serverport,t_serverstate,t_servertype,t_dbName,t_dbip,t_dbport,t_username,t_password,t_servicetel,t_serviceemail,t_serviceqq);
END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_AllSysMsg` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_AllSysMsg` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_AllSysMsg`(pServerID smallint unsigned)
BEGIN
	declare t_id int unsigned default 0;
	select MsgID into t_id from t_sysmsg limit 0, 1;
	if t_id = 0 then
		select 2;
		select 0,0,0,0,0,"",0;
	else
		select 0;
		select MsgID,MsgType,UNIX_TIMESTAMP(BeginTime),UNIX_TIMESTAMP(EndTime),IntervalTime,MsgContent,state from t_sysmsg where (ServerID=pServerID or ServerID = 0) and EndTime > now() and state = 0;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_GDB_Get_SysMsg` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_GDB_Get_SysMsg` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `P_GDB_Get_SysMsg`(pMsgID int unsigned)
BEGIN
	declare t_msgid int unsigned default 0;
	
	select MsgID into t_msgid from t_sysmsg where MsgID = pMsgID;
	if t_msgid = 0 then
		select 2;
		select 0,0,0,0,0,"",0;
	else
		select 0;
		select MsgID,MsgType,UNIX_TIMESTAMP(BeginTime),UNIX_TIMESTAMP(EndTime),IntervalTime,MsgContent,state from t_sysmsg where MsgID = pMsgID;
	end if;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `P_TestProc` */

/*!50003 DROP PROCEDURE IF EXISTS  `P_TestProc` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`localhost` PROCEDURE `P_TestProc`( pgoodsnum smallint unsigned ,pPacketGoodsData blob(10240))
BEGIN
          declare t_count int default 0;
          declare t_uid bigint unsigned default 0;
         declare t_goodsid int unsigned default 0;
         declare CreateTime int unsigned default 0;
         declare Number  tinyint unsigned default 0;
         declare Binded  tinyint default 0;
         declare goodsdata  varbinary (30) default "";
   
         declare pos int default 1;
         while t_count < pgoodsnum do
 
         set t_uid = ToUBigInt(SUBSTRING(pPacketGoodsData,pos,8));
         set pos = pos+8;
         set t_goodsid = ToUSmallInt(SUBSTRING(pPacketGoodsData,pos,2));
         set pos = pos+2;
          set CreateTime = ToUInt(SUBSTRING(pPacketGoodsData,pos,4));
         set pos = pos+4;
          set Number = ASCII(SUBSTRING(pPacketGoodsData,pos,1));
         set pos = pos+1;
          set Binded = ASCII(SUBSTRING(pPacketGoodsData,pos,1));
         set pos = pos+1;
          set goodsdata = SUBSTRING(pPacketGoodsData,pos,14);
           set pos = pos+14;
         insert t_test(uid,goodsid,createTime,number,binded,goodsdata) value (t_uid,t_goodsid,CreateTime,Number,Binded,goodsdata);
          set t_count = t_count + 1;
         end while;
         select t_count;
          
      
    END */$$
DELIMITER ;

/* Procedure structure for procedure `testpro` */

/*!50003 DROP PROCEDURE IF EXISTS  `testpro` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `testpro`()
BEGIN
	declare t_time datetime default 0;
	select donttalkbegin into t_time from t_donttalk where id=1;
	select unix_timestamp(t_time);
    END */$$
DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
