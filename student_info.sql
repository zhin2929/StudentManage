/*
 Navicat Premium Data Transfer

 Source Server         : localhost
 Source Server Type    : MySQL
 Source Server Version : 80033 (8.0.33)
 Source Host           : localhost:3306
 Source Schema         : student_info

 Target Server Type    : MySQL
 Target Server Version : 80033 (8.0.33)
 File Encoding         : 65001

 Date: 26/05/2023 13:08:00
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for class
-- ----------------------------
DROP TABLE IF EXISTS `class`;
CREATE TABLE `class`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `class_id` int NOT NULL,
  `class_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  PRIMARY KEY (`id`, `class_id`) USING BTREE,
  UNIQUE INDEX `class_id`(`class_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 39 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of class
-- ----------------------------
INSERT INTO `class` VALUES (1, 1, '77班');
INSERT INTO `class` VALUES (2, 2, '2班');
INSERT INTO `class` VALUES (17, 3, '3班');
INSERT INTO `class` VALUES (29, 9, '4班');
INSERT INTO `class` VALUES (33, 14, '5班');

-- ----------------------------
-- Table structure for course
-- ----------------------------
DROP TABLE IF EXISTS `course`;
CREATE TABLE `course`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `course_id` int NOT NULL,
  `course_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  PRIMARY KEY (`id`, `course_id`) USING BTREE,
  UNIQUE INDEX `course_id`(`course_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 16 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of course
-- ----------------------------
INSERT INTO `course` VALUES (1, 1, 'C++');
INSERT INTO `course` VALUES (3, 3, 'C++');
INSERT INTO `course` VALUES (4, 4, 'Java');
INSERT INTO `course` VALUES (5, 5, 'C语言');
INSERT INTO `course` VALUES (6, 6, '数据结构');
INSERT INTO `course` VALUES (7, 7, '计算机组成原理');
INSERT INTO `course` VALUES (8, 8, 'C++');
INSERT INTO `course` VALUES (9, 9, '汇编');
INSERT INTO `course` VALUES (10, 10, '逆向');
INSERT INTO `course` VALUES (15, 11, 'CPP11');

-- ----------------------------
-- Table structure for score
-- ----------------------------
DROP TABLE IF EXISTS `score`;
CREATE TABLE `score`  (
  `student_id` int NOT NULL,
  `course_id` int NOT NULL,
  `score` decimal(10, 2) NOT NULL,
  PRIMARY KEY (`student_id`, `course_id`) USING BTREE,
  INDEX `course_id`(`course_id` ASC) USING BTREE,
  INDEX `student_id`(`student_id` ASC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of score
-- ----------------------------
INSERT INTO `score` VALUES (1, 2, 88.00);
INSERT INTO `score` VALUES (2, 1, 78.00);
INSERT INTO `score` VALUES (2, 22, 78.00);
INSERT INTO `score` VALUES (3, 3, 66.00);

-- ----------------------------
-- Table structure for student
-- ----------------------------
DROP TABLE IF EXISTS `student`;
CREATE TABLE `student`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `student_id` int NOT NULL,
  `student_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `class_id` int NOT NULL,
  PRIMARY KEY (`id`, `student_id`) USING BTREE,
  UNIQUE INDEX `student_id`(`student_id` ASC) USING BTREE,
  INDEX `class_id`(`class_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 16 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of student
-- ----------------------------
INSERT INTO `student` VALUES (1, 1, '李2', 5);
INSERT INTO `student` VALUES (2, 2, '李1', 9);
INSERT INTO `student` VALUES (3, 3, '张2', 9);
INSERT INTO `student` VALUES (4, 44, '张2222', 2);
INSERT INTO `student` VALUES (5, 5, '张1', 9);
INSERT INTO `student` VALUES (6, 6, '赵六', 8);
INSERT INTO `student` VALUES (7, 7, '李四', 3);
INSERT INTO `student` VALUES (8, 8, '王五', 2);
INSERT INTO `student` VALUES (10, 10, '小兰', 10);
INSERT INTO `student` VALUES (12, 11, '小明', 5);
INSERT INTO `student` VALUES (13, 9, '张三', 5);

SET FOREIGN_KEY_CHECKS = 1;
