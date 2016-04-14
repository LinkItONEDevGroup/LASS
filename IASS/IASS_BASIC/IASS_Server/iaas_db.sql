-- phpMyAdmin SQL Dump
-- version 4.2.12deb2+deb8u1
-- http://www.phpmyadmin.net
--
-- 主機: localhost
-- 產生時間： 2016 年 04 月 12 日 15:17
-- 伺服器版本: 5.5.44-0+deb8u1
-- PHP 版本： 5.6.19-0+deb8u1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- 資料庫： `sensor_data`
--
CREATE DATABASE IF NOT EXISTS `sensor_data` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `sensor_data`;

DELIMITER $$
--
-- Procedure
--
DROP PROCEDURE IF EXISTS `archive_records`$$
CREATE DEFINER=`root`@`localhost` PROCEDURE `archive_records`()
insert into data_storage2 (sensor_id, datetime, txtdata)
SELECT sensor_id, UNIX_TIMESTAMP(FROM_UNIXTIME(datetime,"%Y-%m-%d %H")) as datetime, avg(convert(txtdata ,UNSIGNED)) as txtdata FROM data_storage where sensor_id in (1,2,3,4,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24) and datetime<=(UNIX_TIMESTAMP(now())-(24*7*60*60)) group by 2,1$$

DROP PROCEDURE IF EXISTS `delete_records`$$
CREATE DEFINER=`root`@`localhost` PROCEDURE `delete_records`()
DELETE FROM data_storage
where sensor_id in (1,2,3,4,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24) and datetime<=(UNIX_TIMESTAMP(now())-(24*7*60*60))$$

DELIMITER ;

-- --------------------------------------------------------

--
-- 資料表結構 `data_storage`
--

DROP TABLE IF EXISTS `data_storage`;
CREATE TABLE IF NOT EXISTS `data_storage` (
`id` int(10) NOT NULL,
  `sensor_id` int(6) NOT NULL,
  `datetime` int(13) NOT NULL,
  `txtdata` varchar(255) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=6098 DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- 資料表結構 `data_storage2`
--

DROP TABLE IF EXISTS `data_storage2`;
CREATE TABLE IF NOT EXISTS `data_storage2` (
`id` int(10) NOT NULL,
  `sensor_id` int(6) NOT NULL,
  `datetime` int(13) NOT NULL,
  `txtdata` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- 資料表結構 `sensor_list`
--

DROP TABLE IF EXISTS `sensor_list`;
CREATE TABLE IF NOT EXISTS `sensor_list` (
`id` int(6) NOT NULL,
  `idname` varchar(30) NOT NULL,
  `name` varchar(60) NOT NULL,
  `moredesc` varchar(250) NOT NULL,
  `type_id` tinyint(3) NOT NULL,
  `create_date` int(13) NOT NULL,
  `alarm_max` int(6) NOT NULL,
  `alarm_min` int(11) NOT NULL,
  `email_time` int(13) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- 資料表結構 `sensor_type`
--

DROP TABLE IF EXISTS `sensor_type`;
CREATE TABLE IF NOT EXISTS `sensor_type` (
`id` tinyint(3) NOT NULL,
  `name` varchar(30) NOT NULL,
  `unit` varchar(12) NOT NULL,
  `maxv` int(8) NOT NULL,
  `minv` int(8) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

--
-- 已匯出資料表的索引
--

--
-- 資料表索引 `data_storage`
--
ALTER TABLE `data_storage`
 ADD PRIMARY KEY (`id`), ADD KEY `sensor_id` (`sensor_id`,`datetime`), ADD KEY `datetime` (`datetime`), ADD KEY `sensor_id_2` (`sensor_id`);

--
-- 資料表索引 `data_storage2`
--
ALTER TABLE `data_storage2`
 ADD PRIMARY KEY (`id`), ADD KEY `sensor_id` (`sensor_id`,`datetime`), ADD KEY `datetime` (`datetime`), ADD KEY `sensor_id_2` (`sensor_id`);

--
-- 資料表索引 `sensor_list`
--
ALTER TABLE `sensor_list`
 ADD PRIMARY KEY (`id`), ADD KEY `idname` (`idname`);

--
-- 資料表索引 `sensor_type`
--
ALTER TABLE `sensor_type`
 ADD PRIMARY KEY (`id`);

--
-- 在匯出的資料表使用 AUTO_INCREMENT
--

--
-- 使用資料表 AUTO_INCREMENT `data_storage`
--
ALTER TABLE `data_storage`
MODIFY `id` int(10) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=6098;
--
-- 使用資料表 AUTO_INCREMENT `data_storage2`
--
ALTER TABLE `data_storage2`
MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- 使用資料表 AUTO_INCREMENT `sensor_list`
--
ALTER TABLE `sensor_list`
MODIFY `id` int(6) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=14;
--
-- 使用資料表 AUTO_INCREMENT `sensor_type`
--
ALTER TABLE `sensor_type`
MODIFY `id` tinyint(3) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=12;

INSERT INTO `sensor_type` (`id`, `name`, `unit`, `maxv`, `minv`) VALUES
(1, '溫度', '°C', 60, 0),
(2, '溼度', '%', 100, 0),
(3, '漏水', '%', 100, 0),
(4, '煙霧', 'ppm', 1000, 0),
(5, '一氧化碳', 'ppm', 1000, 0),
(6, '瓦斯', 'ppm', 2000, 0),
(7, 'PM1.0', 'ug/m3', 100, 0),
(8, 'PM2.5', 'ug/m3', 100, 0),
(9, 'PM10', 'ug/m3', 100, 0),
(10, 'CO2', 'ppm', 2000, 0),
(11, '光照度', 'Lux', 300, 0);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
