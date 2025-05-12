-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Apr 22, 2025 at 05:38 PM
-- Server version: 10.4.32-MariaDB
-- PHP Version: 8.2.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `defaultdb`
--

-- --------------------------------------------------------

--
-- Table structure for table `clans`
--

CREATE TABLE `clans` (
  `id` int(11) NOT NULL,
  `name` varchar(100) NOT NULL,
  `clan_prefix` varchar(10) NOT NULL,
  `clan_logo` varchar(150) DEFAULT NULL,
  `clan_logo_motd` varchar(150) DEFAULT NULL,
  `leader_id` int(11) NOT NULL,
  `bank_balance` int(11) DEFAULT 0,
  `upgrade_level` tinyint(4) DEFAULT 0,
  `last_payment` datetime DEFAULT current_timestamp(),
  `created_at` datetime DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


--
-- Triggers `clans`
--
DELIMITER $$
CREATE TRIGGER `insert_leader` AFTER INSERT ON `clans` FOR EACH ROW INSERT INTO clan_members set user_id=new.leader_id,
clan_id= new.id
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `leader_coins` BEFORE DELETE ON `clans` FOR EACH ROW begin 
IF old.bank_balance > 0 THEN
	INSERT INTO coins_record set user_id= old.leader_id,
	clan_name=old.name,
	balance=old.bank_balance;
END IF;
end
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `prevent_leader_if_in_clan` BEFORE INSERT ON `clans` FOR EACH ROW BEGIN
    DECLARE clan_count INT;

    SELECT COUNT(*) INTO clan_count
    FROM clan_members
    WHERE user_id = NEW.leader_id;

    IF clan_count > 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'لا يمكن تعيين القائد لأنه عضو في عشيرة أخرى.';
    END IF;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `clan_donations`
--

CREATE TABLE `clan_donations` (
  `id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `clan_id` int(11) NOT NULL,
  `amount` int(11) NOT NULL,
  `donated_at` datetime DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


--
-- Triggers `clan_donations`
--
DELIMITER $$
CREATE TRIGGER `check_member_before_donation` BEFORE INSERT ON `clan_donations` FOR EACH ROW BEGIN
    DECLARE is_member INT;

    SELECT COUNT(*) INTO is_member
    FROM clan_members
    WHERE user_id = NEW.user_id AND clan_id = NEW.clan_id;

    IF is_member = 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'لا يمكن التبرع لأن المستخدم ليس عضوًا في العشيرة.';
    END IF;
END
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `update_clan_balance_after_donation` AFTER INSERT ON `clan_donations` FOR EACH ROW BEGIN
    UPDATE clans
    SET bank_balance = bank_balance + NEW.amount
    WHERE id = NEW.clan_id;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Stand-in structure for view `clan_donations_temp`
-- (See below for the actual view)
--
CREATE TABLE `clan_donations_temp` (
`user_id` int(11)
,`clan_id` int(11)
,`total_donations` decimal(32,0)
);

-- --------------------------------------------------------

--
-- Table structure for table `clan_invitations`
--

CREATE TABLE `clan_invitations` (
  `id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `clan_id` int(11) NOT NULL,
  `request_date` datetime DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


--
-- Triggers `clan_invitations`
--
DELIMITER $$
CREATE TRIGGER `check_user_if_in_clan` BEFORE INSERT ON `clan_invitations` FOR EACH ROW BEGIN
    DECLARE clan_count INT;

    SELECT COUNT(*) INTO clan_count
    FROM clan_members
    WHERE user_id = NEW.user_id;

    IF clan_count > 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'لا يمكن طلب انضمام اذ كنت منضم اصلا.';
    END IF;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `clan_members`
--

CREATE TABLE `clan_members` (
  `user_id` int(11) NOT NULL,
  `clan_id` int(11) NOT NULL,
  `joined_at` datetime DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


--
-- Triggers `clan_members`
--
DELIMITER $$
CREATE TRIGGER `check_after_add_new_member` AFTER INSERT ON `clan_members` FOR EACH ROW BEGIN
    DECLARE new_member INT;

    SELECT COUNT(*) INTO new_member
    FROM clan_invitations
    WHERE user_id = new.user_id ;

    IF new_member > 0 THEN
	DELETE from clan_invitations where user_id=new.user_id;
    END IF;
END
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `prevent_leader_from_leaving` BEFORE DELETE ON `clan_members` FOR EACH ROW BEGIN
    DECLARE is_leader INT;

    SELECT COUNT(*) INTO is_leader
    FROM clans
    WHERE leader_id = OLD.user_id AND id = OLD.clan_id;

    IF is_leader > 0 THEN
        SIGNAL SQLSTATE '45000'
        SET MESSAGE_TEXT = 'لا يمكن للقائد مغادرة العشيرة. يجب تعيين قائد جديد أولاً.';
    END IF;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `coins_record`
--

CREATE TABLE `coins_record` (
  `id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `clan_name` varchar(100) NOT NULL,
  `balance` int(11) NOT NULL DEFAULT 0,
  `date_b` datetime DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


-- --------------------------------------------------------

--
-- Stand-in structure for view `inactive_clans`
-- (See below for the actual view)
--
CREATE TABLE `inactive_clans` (
`id` int(11)
,`name` varchar(100)
,`clan_prefix` varchar(10)
,`clan_logo` varchar(150)
,`clan_logo_motd` varchar(150)
,`leader_id` int(11)
,`bank_balance` int(11)
,`upgrade_level` tinyint(4)
,`last_payment` datetime
,`created_at` datetime
);

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `user_id` int(11) NOT NULL,
  `user_name` varchar(40) NOT NULL,
  `steam_id` varchar(40) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;



-- --------------------------------------------------------

--
-- Structure for view `clan_donations_temp`
--
DROP TABLE IF EXISTS `clan_donations_temp`;

CREATE VIEW `clan_donations_temp`  AS SELECT `clan_donations`.`user_id` AS `user_id`, `clan_donations`.`clan_id` AS `clan_id`, sum(`clan_donations`.`amount`) AS `total_donations` FROM `clan_donations` WHERE `clan_donations`.`donated_at` >= current_timestamp() - interval 14 day GROUP BY `clan_donations`.`user_id`, `clan_donations`.`clan_id` ;

-- --------------------------------------------------------

--
-- Structure for view `inactive_clans`
--
DROP TABLE IF EXISTS `inactive_clans`;

CREATE VIEW `inactive_clans`  AS SELECT `clans`.`id` AS `id`, `clans`.`name` AS `name`, `clans`.`clan_prefix` AS `clan_prefix`, `clans`.`clan_logo` AS `clan_logo`, `clans`.`clan_logo_motd` AS `clan_logo_motd`, `clans`.`leader_id` AS `leader_id`, `clans`.`bank_balance` AS `bank_balance`, `clans`.`upgrade_level` AS `upgrade_level`, `clans`.`last_payment` AS `last_payment`, `clans`.`created_at` AS `created_at` FROM `clans` WHERE `clans`.`last_payment` <= current_timestamp() - interval 14 day ;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `clans`
--
ALTER TABLE `clans`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`),
  ADD UNIQUE KEY `clan_prefix` (`clan_prefix`),
  ADD UNIQUE KEY `leader_id` (`leader_id`);

--
-- Indexes for table `clan_donations`
--
ALTER TABLE `clan_donations`
  ADD PRIMARY KEY (`id`),
  ADD KEY `user_id` (`user_id`),
  ADD KEY `clan_id` (`clan_id`);

--
-- Indexes for table `clan_invitations`
--
ALTER TABLE `clan_invitations`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `user_id` (`user_id`,`clan_id`),
  ADD KEY `clan_id` (`clan_id`);

--
-- Indexes for table `clan_members`
--
ALTER TABLE `clan_members`
  ADD UNIQUE KEY `user_id` (`user_id`),
  ADD KEY `clan_id` (`clan_id`);

--
-- Indexes for table `coins_record`
--
ALTER TABLE `coins_record`
  ADD PRIMARY KEY (`id`),
  ADD KEY `user_id` (`user_id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`user_id`),
  ADD UNIQUE KEY `user_name` (`user_name`,`steam_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `clans`
--
ALTER TABLE `clans`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `clan_donations`
--
ALTER TABLE `clan_donations`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `clan_invitations`
--
ALTER TABLE `clan_invitations`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `coins_record`
--
ALTER TABLE `coins_record`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `user_id` int(11) NOT NULL AUTO_INCREMENT;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `clans`
--
ALTER TABLE `clans`
  ADD CONSTRAINT `clans_ibfk_1` FOREIGN KEY (`leader_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE;

--
-- Constraints for table `clan_donations`
--
ALTER TABLE `clan_donations`
  ADD CONSTRAINT `clan_donations_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `clan_members` (`user_id`) ON DELETE CASCADE,
  ADD CONSTRAINT `clan_donations_ibfk_2` FOREIGN KEY (`clan_id`) REFERENCES `clans` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `clan_invitations`
--
ALTER TABLE `clan_invitations`
  ADD CONSTRAINT `clan_invitations_ibfk_1` FOREIGN KEY (`clan_id`) REFERENCES `clans` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `clan_invitations_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE;

--
-- Constraints for table `clan_members`
--
ALTER TABLE `clan_members`
  ADD CONSTRAINT `clan_members_ibfk_1` FOREIGN KEY (`clan_id`) REFERENCES `clans` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `clan_members_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE;

--
-- Constraints for table `coins_record`
--
ALTER TABLE `coins_record`
  ADD CONSTRAINT `coins_record_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE;

DELIMITER $$
--
-- Events
--
CREATE EVENT `delete inctive clan` ON SCHEDULE EVERY 1 MINUTE STARTS '2025-04-11 17:49:44' ON COMPLETION NOT PRESERVE ENABLE DO BEGIN
DELETE from clans where upgrade_level=0 and bank_balance<300 and id in (SELECT id from inactive_clans);

UPDATE clans 
SET bank_balance = bank_balance - 
    CASE 
		WHEN upgrade_level = 0 THEN 300
        WHEN upgrade_level = 1 THEN 500
        WHEN upgrade_level = 2 THEN 700
        ELSE 0
    END,
	last_payment = NOW()
where (
(upgrade_level=0 and bank_balance>=300) or 
(upgrade_level=1 and bank_balance>=500) or
(upgrade_level=2 and bank_balance>=700)) and id in (SELECT id from inactive_clans);

UPDATE clans 
SET upgrade_level = upgrade_level - 1,
 last_payment = NOW()
 where upgrade_level>0 and (
(upgrade_level=1 and bank_balance<500) or
(upgrade_level=2 and bank_balance<700)) and id in (SELECT id from inactive_clans);


end$$

DELIMITER ;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
