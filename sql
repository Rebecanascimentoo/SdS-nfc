CREATE DATABASE  SecuritySystem;
USE SecuritySystem;
CREATE TABLE Students (
    id INT PRIMARY KEY AUTO_INCREMENT,
    uid CHAR (16) NOT NULL,
    name CHAR (100) NOT NULL,
    series CHAR (10) NOT NULL,
    cpf CHAR (14) NOT NULL,
    course CHAR (100) NOT NULL
);