CODIGO SQL:

CREATE DATABASE IF NOT EXISTS SecuritySystem;
USE SecuritySystem;

CREATE TABLE Students (
    id INT PRIMARY KEY AUTO_INCREMENT,
    uid VARCHAR(16) NOT NULL,
    name VARCHAR(100) NOT NULL,
    series VARCHAR(10) NOT NULL,
    cpf VARCHAR(14) NOT NULL,
    course VARCHAR(100) NOT NULL
);

CREATE INDEX idx_uid ON Students (uid);

INSERT INTO Students (uid, name, series, cpf, course) VALUES 
('abcdef12', 'Alice Silva', '3A', '123.456.789-00', 'Mecatronica'),
('bcdef123', 'Bruno Souza', '2B', '234.567.890-11', 'Logistica'),
('cdef1234', 'Carla Pereira', '1C', '345.678.901-22', 'Mecanica'),
('def12345', 'Daniel Lima', '3A', '456.789.012-33', 'Mecatronica'),
('ef123456', 'Eduardo Costa', '2B', '567.890.123-44', 'Logistica'),
('f1234567', 'Fernanda Nunes', '1C', '678.901.234-55', 'Mecanica'),
('01234567', 'Gustavo Rocha', '3A', '789.012.345-66', 'Mecatronica'),
('12345678', 'Heloisa Martins', '2B', '890.123.456-77', 'Logistica'),
('23456789', 'Isabela Oliveira', '1C', '901.234.567-88', 'Mecanica'),
('34567890', 'Júlio Fernandes', '3A', '012.345.678-99', 'Mecatronica');
