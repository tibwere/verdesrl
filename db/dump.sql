-- MySQL Workbench Forward Engineering

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';

-- -----------------------------------------------------
-- Schema verdesrl
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS `verdesrl` ;

-- -----------------------------------------------------
-- Schema verdesrl
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `verdesrl` ;
USE `verdesrl` ;

-- -----------------------------------------------------
-- Table `verdesrl`.`utenti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`utenti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`utenti` (
  `username` VARCHAR(128) NOT NULL,
  `password` CHAR(128) NOT NULL,
  `ruolo` ENUM("PCS", "RCS", "WHC", "OPC", "MNG", "COS") NOT NULL,
  `uuid` CHAR(36) NOT NULL,
  PRIMARY KEY (`username`),
  UNIQUE INDEX `uuid_UNIQUE` (`uuid` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`specie_di_piante`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`specie_di_piante` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`specie_di_piante` (
  `codice` INT NOT NULL AUTO_INCREMENT,
  `nome_latino` VARCHAR(64) NOT NULL,
  `nome_comune` VARCHAR(64) NOT NULL,
  `interni_or_esterni` TINYINT NOT NULL,
  `verde_or_fiorita` TINYINT NOT NULL,
  `esotica_si_no` TINYINT NOT NULL,
  `giacenza` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`codice`),
  UNIQUE INDEX `nome_latino_UNIQUE` (`nome_latino` ASC),
  FULLTEXT INDEX `nome_comune_FULLTEXT` (`nome_comune`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`prezzi`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`prezzi` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`prezzi` (
  `specie_di_pianta` INT NOT NULL,
  `data_adozione` DATETIME NOT NULL,
  `valore` DECIMAL(7,2) UNSIGNED NOT NULL,
  `data_termine` DATETIME NULL,
  PRIMARY KEY (`specie_di_pianta`, `data_adozione`),
  INDEX `fk_prezzi_specie_di_piante1_idx` (`specie_di_pianta` ASC),
  CONSTRAINT `fk_prezzi_specie_di_piante1`
    FOREIGN KEY (`specie_di_pianta`)
    REFERENCES `verdesrl`.`specie_di_piante` (`codice`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`contatti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`contatti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`contatti` (
  `valore` VARCHAR(256) NOT NULL,
  `tipo` ENUM("telefono", "cellulare", "email") NOT NULL,
  PRIMARY KEY (`valore`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`clienti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`clienti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`clienti` (
  `codice` VARCHAR(16) NOT NULL,
  `nome` VARCHAR(32) NOT NULL,
  `indirizzo_residenza` VARCHAR(64) NOT NULL,
  `indirizzo_fatturazione` VARCHAR(64) NULL,
  `privato_or_rivendita` TINYINT NOT NULL,
  `contatto_preferito` VARCHAR(256) NULL,
  PRIMARY KEY (`codice`),
  INDEX `fk_clienti_contatti1_idx` (`contatto_preferito` ASC),
  CONSTRAINT `fk_clienti_contatti1`
    FOREIGN KEY (`contatto_preferito`)
    REFERENCES `verdesrl`.`contatti` (`valore`)
    ON DELETE SET NULL
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`referenti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`referenti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`referenti` (
  `rivendita` VARCHAR(16) NOT NULL,
  `nome` VARCHAR(32) NOT NULL,
  `cognome` VARCHAR(32) NOT NULL,
  `contatto_preferito` VARCHAR(256) NULL,
  INDEX `fk_referenti_contatti1_idx` (`contatto_preferito` ASC),
  PRIMARY KEY (`rivendita`),
  CONSTRAINT `fk_referenti_contatti1`
    FOREIGN KEY (`contatto_preferito`)
    REFERENCES `verdesrl`.`contatti` (`valore`)
    ON DELETE SET NULL
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_referenti_clienti1`
    FOREIGN KEY (`rivendita`)
    REFERENCES `verdesrl`.`clienti` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`ordini`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`ordini` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`ordini` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `cliente` VARCHAR(16) NOT NULL,
  `data` DATETIME NOT NULL,
  `aperto_or_finalizzato` TINYINT NOT NULL DEFAULT 1,
  `indirizzo_spedizione` VARCHAR(64) NOT NULL,
  `contatto` VARCHAR(256) NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_ordini_contatti1_idx` (`contatto` ASC),
  INDEX `fk_ordini_clienti1_idx` (`cliente` ASC),
  CONSTRAINT `fk_ordini_contatti1`
    FOREIGN KEY (`contatto`)
    REFERENCES `verdesrl`.`contatti` (`valore`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_ordini_clienti1`
    FOREIGN KEY (`cliente`)
    REFERENCES `verdesrl`.`clienti` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`pacchi`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`pacchi` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`pacchi` (
  `ordine` INT NOT NULL,
  `numero` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`ordine`, `numero`),
  INDEX `fk_pacchi_ordini1_idx` (`ordine` ASC),
  CONSTRAINT `fk_pacchi_ordini1`
    FOREIGN KEY (`ordine`)
    REFERENCES `verdesrl`.`ordini` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`fornitori`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`fornitori` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`fornitori` (
  `codice_fornitore` INT NOT NULL AUTO_INCREMENT,
  `codice_fiscale` CHAR(16) NOT NULL,
  `nome` VARCHAR(32) NOT NULL,
  PRIMARY KEY (`codice_fornitore`),
  UNIQUE INDEX `codice_fiscale_UNIQUE` (`codice_fiscale` ASC))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`indirizzi_fornitori`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`indirizzi_fornitori` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`indirizzi_fornitori` (
  `indirizzo` VARCHAR(64) NOT NULL,
  `fornitore` INT NOT NULL,
  PRIMARY KEY (`indirizzo`),
  INDEX `fk_indirizzi_fornitori_fornitori1_idx` (`fornitore` ASC),
  CONSTRAINT `fk_indirizzi_fornitori_fornitori1`
    FOREIGN KEY (`fornitore`)
    REFERENCES `verdesrl`.`fornitori` (`codice_fornitore`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`richieste_di_forniture`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`richieste_di_forniture` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`richieste_di_forniture` (
  `data` DATETIME NOT NULL,
  `fornitore_scelto` INT NOT NULL,
  `specie_richiesta` INT NOT NULL,
  `quantita` INT UNSIGNED NOT NULL DEFAULT 0,
  `pendente_si_no` TINYINT NOT NULL DEFAULT 1,
  PRIMARY KEY (`data`, `fornitore_scelto`, `specie_richiesta`),
  INDEX `fk_richieste_di_forniture_specie_di_piante1_idx` (`specie_richiesta` ASC),
  INDEX `fk_richieste_di_forniture_fornitori1_idx` (`fornitore_scelto` ASC),
  CONSTRAINT `fk_richieste_di_forniture_specie_di_piante1`
    FOREIGN KEY (`specie_richiesta`)
    REFERENCES `verdesrl`.`specie_di_piante` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_richieste_di_forniture_fornitori1`
    FOREIGN KEY (`fornitore_scelto`)
    REFERENCES `verdesrl`.`fornitori` (`codice_fornitore`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`colorazioni`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`colorazioni` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`colorazioni` (
  `specie_fiorita` INT NOT NULL,
  `colore` VARCHAR(32) NOT NULL,
  PRIMARY KEY (`specie_fiorita`, `colore`),
  CONSTRAINT `fk_colorazioni_specie_di_piante1`
    FOREIGN KEY (`specie_fiorita`)
    REFERENCES `verdesrl`.`specie_di_piante` (`codice`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`contatti_clienti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`contatti_clienti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`contatti_clienti` (
  `contatto` VARCHAR(256) NOT NULL,
  `cliente` VARCHAR(16) NOT NULL,
  PRIMARY KEY (`contatto`),
  INDEX `fk_contatti_clienti_clienti1_idx` (`cliente` ASC),
  CONSTRAINT `fk_contatti_clienti_contatti1`
    FOREIGN KEY (`contatto`)
    REFERENCES `verdesrl`.`contatti` (`valore`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_contatti_clienti_clienti1`
    FOREIGN KEY (`cliente`)
    REFERENCES `verdesrl`.`clienti` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`contatti_referenti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`contatti_referenti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`contatti_referenti` (
  `contatto` VARCHAR(256) NOT NULL,
  `referente` VARCHAR(16) NOT NULL,
  PRIMARY KEY (`contatto`),
  INDEX `fk_contatti_referenti_referenti1_idx` (`referente` ASC),
  CONSTRAINT `fk_contatti_referenti_contatti1`
    FOREIGN KEY (`contatto`)
    REFERENCES `verdesrl`.`contatti` (`valore`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_contatti_referenti_referenti1`
    FOREIGN KEY (`referente`)
    REFERENCES `verdesrl`.`referenti` (`rivendita`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`contenuto_pacchi`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`contenuto_pacchi` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`contenuto_pacchi` (
  `ordine` INT NOT NULL,
  `pacco` INT UNSIGNED NOT NULL,
  `specie_di_pianta` INT NOT NULL,
  `quantita` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ordine`, `pacco`, `specie_di_pianta`),
  INDEX `fk_contenuto_pacchi_specie_di_piante1_idx` (`specie_di_pianta` ASC),
  INDEX `fk_contenuto_pacchi_pacchi1_idx` (`ordine` ASC, `pacco` ASC),
  CONSTRAINT `fk_contenuto_pacchi_specie_di_piante1`
    FOREIGN KEY (`specie_di_pianta`)
    REFERENCES `verdesrl`.`specie_di_piante` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_contenuto_pacchi_pacchi1`
    FOREIGN KEY (`ordine` , `pacco`)
    REFERENCES `verdesrl`.`pacchi` (`ordine` , `numero`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`disponibilita_forniture`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`disponibilita_forniture` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`disponibilita_forniture` (
  `specie_di_pianta` INT NOT NULL,
  `fornitore` INT NOT NULL,
  PRIMARY KEY (`specie_di_pianta`, `fornitore`),
  INDEX `fk_disponibilta_forniture_specie_di_piante1_idx` (`specie_di_pianta` ASC),
  INDEX `fk_disponibilta_forniture_fornitori1_idx` (`fornitore` ASC),
  CONSTRAINT `fk_disponibilta_forniture_specie_di_piante1`
    FOREIGN KEY (`specie_di_pianta`)
    REFERENCES `verdesrl`.`specie_di_piante` (`codice`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_disponibilta_forniture_fornitori1`
    FOREIGN KEY (`fornitore`)
    REFERENCES `verdesrl`.`fornitori` (`codice_fornitore`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`utenze_clienti`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`utenze_clienti` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`utenze_clienti` (
  `cliente` VARCHAR(128) NOT NULL,
  `utente` VARCHAR(32) NOT NULL,
  PRIMARY KEY (`cliente`),
  INDEX `fk_utenze_clienti_utenti1_idx` (`utente` ASC),
  CONSTRAINT `fk_utenze_clienti_clienti1`
    FOREIGN KEY (`cliente`)
    REFERENCES `verdesrl`.`clienti` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_utenze_clienti_utenti1`
    FOREIGN KEY (`utente`)
    REFERENCES `verdesrl`.`utenti` (`username`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `verdesrl`.`appartenenza_ordini`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `verdesrl`.`appartenenza_ordini` ;

CREATE TABLE IF NOT EXISTS `verdesrl`.`appartenenza_ordini` (
  `ordine` INT NOT NULL,
  `specie_di_pianta` INT NOT NULL,
  `quantita` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ordine`, `specie_di_pianta`),
  INDEX `fk_appartenenza_ordini_specie_di_piante1_idx` (`specie_di_pianta` ASC),
  INDEX `fk_appartenenza_ordini_ordini1_idx` (`ordine` ASC),
  CONSTRAINT `fk_appartenenza_ordini_specie_di_piante1`
    FOREIGN KEY (`specie_di_pianta`)
    REFERENCES `verdesrl`.`specie_di_piante` (`codice`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_appartenenza_ordini_ordini1`
    FOREIGN KEY (`ordine`)
    REFERENCES `verdesrl`.`ordini` (`id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;

USE `verdesrl` ;

-- -----------------------------------------------------
-- procedure crea_utenza_dipendente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`crea_utenza_dipendente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `crea_utenza_dipendente` (
	IN var_username VARCHAR(128), 
    IN var_password VARCHAR(128), 
    IN var_ruolo 	CHAR(3)
)
BEGIN
	DECLARE var_uuid CHAR(36);
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45011' SET MESSAGE_TEXT = "User already exists";
        ELSE
			RESIGNAL;  
        END IF;
    END;

	-- La transazione e' stata utilizzata in questo caso per
    -- effettuare un insieme di scritture in modalita'
    -- "all or nothing" non e' quindi necessario fissare
    -- un livello di isolamento.
    START TRANSACTION;
    
		IF var_ruolo = "PCS" OR var_ruolo = "RCS" THEN
			SIGNAL SQLSTATE '45033' SET MESSAGE_TEXT = "Selected role is not an employee";
        END IF;
    
		SELECT UUID() INTO var_uuid;
    
		IF verifica_password(var_password) = 1 THEN
			SIGNAL SQLSTATE '45003' SET MESSAGE_TEXT = "Minimum password length is 8 characters";
		ELSEIF verifica_password(var_password) = 2 THEN
			SIGNAL SQLSTATE '45004' SET MESSAGE_TEXT = "Password must not contains spaces";
		ELSE
			INSERT INTO utenti (`username`, `password`, `ruolo`, `uuid`) 
				VALUES (var_username, SHA2(CONCAT(var_password, var_uuid), 512), var_ruolo, var_uuid);
		END IF;
        
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_password
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_password`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_password` (
	IN var_username 		VARCHAR(32),
    IN var_vecchia_password VARCHAR(128),
	IN var_nuova_password 	VARCHAR(128)
)
BEGIN
	DECLARE var_vecchia_password_memorizzata VARCHAR(128);
    DECLARE var_uuid CHAR(36);
    
	DECLARE CONTINUE HANDLER FOR NOT FOUND SIGNAL SQLSTATE '45028' SET MESSAGE_TEXT = "User not exists";
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;

	-- Il livello di isolamento scelto e' READ COMMITTED per via
    -- del fatto che mediante tale stored procedure e' possibile modificare
    -- la password associata ad un'account e per effettuare un confronto 
    -- corretto e' necessario leggere un dato gia' committato
	SET TRANSACTION ISOLATION LEVEL READ COMMITTED;
    START TRANSACTION;
		SELECT	`password`, `uuid`
		FROM 	`utenti`
		WHERE 	`username` = var_username
		INTO 	var_vecchia_password_memorizzata, var_uuid;
		
		IF var_vecchia_password_memorizzata <> SHA2(CONCAT(var_vecchia_password, var_uuid), 512) THEN
			SIGNAL SQLSTATE '45017' SET MESSAGE_TEXT = 'Old password inserted does not match with the stored one';
		END IF;
		
		UPDATE	`utenti` 
        SET 	`password` = SHA2(CONCAT(var_nuova_password, var_uuid), 512) 
        WHERE 	`username` = var_username;
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure login
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`login`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `login` (
	IN 	var_username 		VARCHAR(128), 
    IN 	var_password 		VARCHAR(128), 
    OUT var_ruolo 			INT, 
    OUT var_codice_cliente 	VARCHAR(16)
)
BEGIN
	DECLARE var_ruolo_enum ENUM("PCS", "RCS", "WHC", "OPC", "MNG", "COS", "ERR");
    
	-- l'handler e' necessario per evitare il warning 
    -- 1329: No data - zero rows fetched, selected, or processed
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET var_ruolo_enum = "ERR";
    
	SELECT	ruolo 
    FROM 	utenti
	WHERE 	`username` = var_username and 
			`password` = SHA2(CONCAT(var_password, `uuid`), 512)
    INTO 	var_ruolo_enum;
	

    IF var_ruolo_enum = "PCS" OR var_ruolo_enum = "RCS" THEN
		SELECT	cliente
        FROM	utenze_clienti
        WHERE 	utente = var_username
        INTO	var_codice_cliente;
    END IF;
      
	CASE var_ruolo_enum
		WHEN "PCS" THEN SET var_ruolo = 0;
        WHEN "RCS" THEN SET var_ruolo = 1;
        WHEN "WHC" THEN SET var_ruolo = 2;
        WHEN "OPC" THEN SET var_ruolo = 3;
        WHEN "MNG" THEN SET var_ruolo = 4;
        WHEN "COS" THEN SET var_ruolo = 5;
        ELSE SET var_ruolo = 6;
	END CASE;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure registra_privato
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`registra_privato`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `registra_privato` (
	IN var_codice 	CHAR(16), 
    IN var_nome 	VARCHAR(32), 
    IN var_res 		VARCHAR(64), 
    IN var_fat 		VARCHAR(64),  
    IN var_user 	VARCHAR(128), 
    IN var_pass 	VARCHAR(128)
)
BEGIN
	DECLARE var_uuid CHAR(36);
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN   
		ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45011' SET MESSAGE_TEXT = "User already exists";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	-- La transazione e' stata utilizzata in questo caso per
    -- effettuare un insieme di scritture in modalita'
    -- "all or nothing" non e' quindi necessario fissare
    -- un livello di isolamento.
    START TRANSACTION;
        
        SELECT UUID() INTO var_uuid;
        
		IF verifica_password(var_pass) = 1 THEN
			SIGNAL SQLSTATE '45003' SET MESSAGE_TEXT = "Minimum password length is 8 characters";
		ELSEIF verifica_password(var_pass) = 2 THEN
			SIGNAL SQLSTATE '45004' SET MESSAGE_TEXT = "Password must not contains spaces";
		ELSE
			INSERT INTO utenti (`username`, `password`, `ruolo`, `uuid`) 
				VALUES (var_user, SHA2(CONCAT(var_pass, var_uuid), 512), "PCS", var_uuid);
			INSERT INTO clienti (`codice`, `nome`, `indirizzo_residenza`, `indirizzo_fatturazione`, `privato_or_rivendita`)
				VALUES (UPPER(var_codice), var_nome, var_res, var_fat, 1);
			INSERT INTO utenze_clienti (`utente`, `cliente`) VALUES (var_user, UPPER(var_codice));
        END IF;
        
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure registra_rivendita
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`registra_rivendita`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `registra_rivendita` (	
	IN var_codice 		CHAR(11), 
	IN var_nome_riv 	VARCHAR(32), 
	IN var_res 			VARCHAR(64), 
	IN var_fat 			VARCHAR(64), 
	IN var_user 		VARCHAR(128), 
	IN var_pass 		VARCHAR(128),
	IN var_nome_ref 	VARCHAR(32),
	IN var_cognome_ref 	VARCHAR(32)
)
BEGIN
	DECLARE var_uuid CHAR(36);
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
		ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45011' SET MESSAGE_TEXT = "User already exists";
        ELSE
			RESIGNAL; 
        END IF;
    END;

    START TRANSACTION;
		
        SELECT UUID() INTO var_uuid;
        
        IF verifica_password(var_pass) = 1 THEN
			SIGNAL SQLSTATE '45003' SET MESSAGE_TEXT = "Minimum password length is 8 characters";
		ELSEIF verifica_password(var_pass) = 2 THEN
			SIGNAL SQLSTATE '45004' SET MESSAGE_TEXT = "Password must not contains spaces";
		ELSE
			INSERT INTO utenti (`username`, `password`, `ruolo`, `uuid`) 
				VALUES (var_user, SHA2(CONCAT(var_pass, var_uuid), 512), "RCS", var_uuid);
			INSERT INTO clienti (`codice`, `nome`, `indirizzo_residenza`, `indirizzo_fatturazione`, `privato_or_rivendita`)
				VALUES (var_codice, var_nome_riv, var_res, var_fat, 0);
			INSERT INTO utenze_clienti (`utente`, `cliente`) VALUES (var_user, var_codice);
			INSERT INTO referenti (`rivendita`, `nome`, `cognome`) VALUES (var_codice, var_nome_ref, var_cognome_ref);
		END IF;
        
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure crea_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`crea_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `crea_ordine` (
	IN var_cliente	VARCHAR(16), 
    IN var_ind_sped	VARCHAR(64),
    IN var_contatto VARCHAR(256),
    IN var_specie 	INT, 
	IN var_quantita INT,
    OUT var_id 		INT
)
BEGIN
	DECLARE var_errno INT;
	DECLARE var_verifica_proprietario INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45014' SET MESSAGE_TEXT = "Selected species does not exists";
        ELSE
			RESIGNAL;  
        END IF;
    END;

	DROP TABLE IF EXISTS `verifica_proprietario`;
	CREATE TEMPORARY TABLE `verifica_proprietario` (
		`Contatto`	VARCHAR(256),
		`Cliente`	CHAR(16),
		`Referente`	CHAR(11)
	);

	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
    START TRANSACTION;
		
        IF var_ind_sped IS NULL THEN
			SELECT	indirizzo_residenza
            FROM 	clienti
            WHERE 	codice = var_cliente
            INTO 	var_ind_sped;
		END IF;
        
        IF var_contatto IS NULL THEN
			SELECT 	contatto_preferito
            FROM 	clienti
            WHERE 	codice = var_cliente
            INTO 	var_contatto;
            
            IF var_contatto IS NULL THEN
				SIGNAL SQLSTATE '45002' SET MESSAGE_TEXT  = "Favourite contact not set for current user";
            END IF;
		ELSE
			INSERT INTO `verifica_proprietario`
			SELECT	contatti.valore 				AS `Contatto`,
					contatti_clienti.cliente		AS `Cliente`,
					contatti_referenti.referente	AS `Referente`
			FROM 	contatti 	LEFT JOIN contatti_clienti 		ON contatti_clienti.contatto = valore
								LEFT JOIN contatti_referenti 	ON contatti_referenti.contatto = valore
			WHERE	contatti.valore = var_contatto;
                                
			SELECT	COUNT(*)
			FROM 	`verifica_proprietario`
			WHERE 	(`Cliente` = var_cliente OR `Referente` = var_cliente)
            INTO	var_verifica_proprietario;
            
			IF var_verifica_proprietario = 0 THEN
				SIGNAL SQLSTATE '45009' SET MESSAGE_TEXT = "Neither customer nor referent (if exists) own this contact";
			END IF;
        END IF;
        
		DROP TEMPORARY TABLE `verifica_proprietario`;
    
        INSERT INTO ordini (`cliente`, `data`, `aperto_or_finalizzato`, `indirizzo_spedizione`, `contatto`)
			VALUES (var_cliente, NOW(), 1, var_ind_sped, var_contatto);
            
		SELECT MAX(id) FROM ordini INTO var_id;
        
		INSERT INTO appartenenza_ordini (`specie_di_pianta`, `ordine`, `quantita`) VALUES (var_specie, var_id, var_quantita);
    
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_specie_ad_ordine_esistente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_specie_ad_ordine_esistente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_specie_ad_ordine_esistente` (
	IN var_cliente VARCHAR(16),
	IN var_specie	INT,
    IN var_ordine	INT,
    IN var_quantita INT
)
BEGIN 
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45008' SET MESSAGE_TEXT = "Species already in order";
        ELSEIF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45014' SET MESSAGE_TEXT = "Species does not exists";
        ELSE
			RESIGNAL;  
        END IF;
    END;
    
	START TRANSACTION; 
    
		IF verifica_proprietario(var_ordine, var_cliente) THEN
			INSERT INTO appartenenza_ordini (`specie_di_pianta`, `ordine`, `quantita`) VALUES (var_specie, var_ordine, var_quantita);
		ELSE
			SIGNAL SQLSTATE '45005' SET MESSAGE_TEXT = "Only owners can make changes to an order";
		END IF;
        
	COMMIT;    
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure rimuovi_specie_da_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`rimuovi_specie_da_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `rimuovi_specie_da_ordine` (
	IN var_cliente VARCHAR(16),
    IN var_specie INT,
    IN var_ordine INT,
    OUT var_ordine_eliminato_si_no INT,
    OUT var_eliminazione_eff INT
)
BEGIN
	DECLARE var_capienza_ordine INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;

	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
	START TRANSACTION;
    
		IF verifica_proprietario(var_ordine, var_cliente) THEN
			
            SELECT	COUNT(*)
			FROM	appartenenza_ordini
			WHERE 	specie_di_pianta = var_specie AND
					ordine = var_ordine
			INTO 	var_eliminazione_eff;
				
            IF var_eliminazione_eff = 1 THEN
				DELETE FROM appartenenza_ordini WHERE specie_di_pianta = var_specie AND ordine = var_ordine;				
				
                SELECT	COUNT(*)
				FROM	appartenenza_ordini
				WHERE	ordine = var_ordine
				INTO	var_capienza_ordine;
	
				IF var_capienza_ordine = 0 THEN
					DELETE FROM ordini WHERE id = var_ordine;
					SET var_ordine_eliminato_si_no = 1;
				ELSE
					SET var_ordine_eliminato_si_no = 0;
				END IF;
			END IF;	
        ELSE
			SIGNAL SQLSTATE '45005' SET MESSAGE_TEXT = "Only owners can make changes to an order";
		END IF;
		        
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_ordine` (
	IN var_cliente				VARCHAR(16),
	IN var_specie 				INT,
    IN var_ordine 				INT,
    IN var_quantita 			INT,
    OUT var_aggiornamento_eff	INT
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	START TRANSACTION; 
    
		IF verifica_proprietario(var_ordine, var_cliente) THEN
			SELECT	COUNT(*)
			FROM	appartenenza_ordini
			WHERE 	specie_di_pianta = var_specie AND
					ordine = var_ordine
			INTO 	var_aggiornamento_eff;
            
            IF var_aggiornamento_eff = 1 THEN
				UPDATE 	appartenenza_ordini
				SET 	quantita = var_quantita
				WHERE 	specie_di_pianta = var_specie AND
						ordine = var_ordine;
			END IF;
		ELSE
			SIGNAL SQLSTATE '45005' SET MESSAGE_TEXT = "Only owners can make changes to an order";
		END IF;
        
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure finalizza_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`finalizza_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `finalizza_ordine` (
	IN var_cliente VARCHAR(16),
    IN var_ordine INT
)
BEGIN
	IF verifica_proprietario(var_ordine, var_cliente) THEN      
		UPDATE 	ordini
		SET 	aperto_or_finalizzato = 0
		WHERE 	id = var_ordine;
	ELSE
		SIGNAL SQLSTATE '45005' SET MESSAGE_TEXT = "Only owners can make changes to an order";
	END IF;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure report_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`report_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `report_ordine` (
	IN var_cliente VARCHAR(16),
    IN var_ordine INT
)
BEGIN
    DECLARE var_codice_cursor INT;
    DECLARE var_verifica_esistenza INT DEFAULT 0;
    DECLARE var_data_ordine DATETIME;
    DECLARE var_numero_piante INT;
    
	DECLARE done INT DEFAULT FALSE;
    DECLARE cur CURSOR FOR
		SELECT	codice 
        FROM 	specie_di_piante INNER JOIN appartenenza_ordini ON codice = specie_di_pianta
        WHERE 	ordine = var_ordine;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;

	DROP TEMPORARY TABLE IF EXISTS dettagli_specie;
    CREATE TEMPORARY TABLE dettagli_specie (
		`Code` INT,
        `Name` VARCHAR(131), -- NOME COMUNE (64) ~ NOME LATINO (64) ~ CARATTERI EXTRA (3) 
        `Unit_price_(E)` DECIMAL(7, 2),
        `Required_amount` INT,
        `Cumulative_price_(E)` DECIMAL(8, 2),
        PRIMARY KEY (`Code`)
	);

	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
    START TRANSACTION;
    
		IF NOT verifica_proprietario(var_ordine, var_cliente) THEN
			SIGNAL SQLSTATE '45005' SET MESSAGE_TEXT = "Only owners can view order reports";
        END IF;
    
		SELECT	COUNT(*)
        FROM	ordini
        WHERE	id = var_ordine
        INTO	var_verifica_esistenza;
        
        IF var_verifica_esistenza = 0 THEN
			SIGNAL SQLSTATE '45016' SET MESSAGE_TEXT = "Not existent order";
		END IF;
        
        -- utilizzo la data per selezionare
		-- il prezzo corretto dallo storico
		SELECT	`data`
        FROM	`ordini`
        WHERE	`id` = var_ordine
        INTO 	var_data_ordine;
        
        OPEN cur;
		read_loop: LOOP
			FETCH cur INTO var_codice_cursor;
			IF done THEN
				LEAVE read_loop;
			END IF;
            
            INSERT INTO dettagli_specie 
				SELECT		specie_di_piante.codice															AS	`Code`,
							CONCAT(specie_di_piante.nome_comune, " (", specie_di_piante.nome_latino, + ")")	AS	`Name`,
							prezzi.valore																	AS	`Unit_price_(E)`,
							appartenenza_ordini.quantita													AS	`Required_amount`,
							prezzi.valore * appartenenza_ordini.quantita									AS	`Cumulative_price_(E)`
				FROM		specie_di_piante	INNER JOIN	prezzi				ON	specie_di_piante.codice = prezzi.specie_di_pianta
												INNER JOIN	appartenenza_ordini	ON	specie_di_piante.codice = appartenenza_ordini.specie_di_pianta
				WHERE 		specie_di_piante.codice = var_codice_cursor  AND
							data_adozione <= var_data_ordine
				ORDER BY 	data_adozione DESC
                LIMIT 		1;
        END LOOP;
		CLOSE cur;
        
		SELECT		`ordini`.`id`														AS	`ID`,											
					DATE(`ordini`.`data`)												AS	`Date`,
					`ordini`.`indirizzo_spedizione`										AS	`Shipping_address`,
					`ordini`.`contatto`													AS	`Chosen_contact`,
					CASE
						WHEN `ordini`.`aperto_or_finalizzato` = 1 THEN "Open"
						ELSE "Finalized"
					END 																AS `Status`,
                    CASE 
						WHEN `clienti`.`privato_or_rivendita` = 1 THEN "Not expected"
                        ELSE concat(`referenti`.`nome`, " ", `referenti`.`cognome`)
					END 																AS `Referent`
		FROM		`ordini` 	INNER JOIN 	`clienti`	ON `ordini`.`cliente` = `clienti`.`codice`
                                LEFT JOIN	`referenti`	ON `clienti`.`codice` = `referenti`.`rivendita`	
		WHERE		`id` = var_ordine;
        
        SELECT * FROM dettagli_specie;
        
        SELECT	SUM(`Required_amount`) AS `Number_of_plants`,
				SUM(`Cumulative_price_(E)`) AS `Total_spending`
		FROM	`dettagli_specie`;
        
        DROP TEMPORARY TABLE dettagli_specie;
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure inserisci_nuova_specie
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`inserisci_nuova_specie`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `inserisci_nuova_specie` (
    IN	var_nome_comune VARCHAR(64), 
    IN 	var_nome_latino VARCHAR(64), 
    IN 	var_int_est 	TINYINT, 
    IN 	var_colorazione VARCHAR(32), 
    IN 	var_esotica 	TINYINT, 
    IN 	var_giacenza 	INT, 
    IN 	var_prezzo 		DECIMAL(7, 2),
    OUT var_codice 		INT
)
BEGIN
	DECLARE var_verde_or_fiorita TINYINT;
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45021' SET MESSAGE_TEXT = "Species already stored";
        ELSE
			RESIGNAL;  
        END IF;
    END;    

    SET TRANSACTION ISOLATION LEVEL SERIALIZABLE; 
    START TRANSACTION;

		IF var_colorazione IS NULL THEN
			SET var_verde_or_fiorita = 1;
		ELSE	
			SET var_verde_or_fiorita = 0;
		END IF;
        
		INSERT INTO specie_di_piante (nome_comune, nome_latino, interni_or_esterni, verde_or_fiorita, esotica_si_no, giacenza) 
			VALUES (var_nome_comune, var_nome_latino, var_int_est, var_verde_or_fiorita, var_esotica, var_giacenza);

		SELECT MAX(codice) FROM specie_di_piante INTO var_codice;

		IF var_colorazione IS NOT NULL THEN
			INSERT INTO colorazioni (specie_fiorita, colore) VALUES (var_codice, var_colorazione);
		END IF;
                
        INSERT INTO prezzi (specie_di_pianta, data_adozione, valore) VALUES (var_codice, NOW(), var_prezzo);

    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_prezzo
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_prezzo`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_prezzo` (
	IN var_specie INT,
	IN var_prezzo DECIMAL(7, 2),
    OUT var_aggiornamento_effettivo INT
)
BEGIN
	DECLARE var_nuova_data DATETIME;
    DECLARE var_prezzo_attuale DECIMAL(7, 2);
    DECLARE var_specie_non_trovata TINYINT DEFAULT 0;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
    DECLARE CONTINUE HANDLER FOR NOT FOUND SET var_specie_non_trovata = 1;

	SET TRANSACTION ISOLATION LEVEL READ COMMITTED;
    START TRANSACTION;
    
		SELECT	valore
        FROM 	prezzi
        WHERE	specie_di_pianta = var_specie AND
				data_termine IS NULL
		INTO	var_prezzo_attuale;
        
        IF var_specie_non_trovata = 1 THEN
			SIGNAL SQLSTATE '45014' SET MESSAGE_TEXT = "Species does not exists";
        END IF;
        
        IF var_prezzo_attuale <> var_prezzo THEN

			SET var_nuova_data = NOW();
			
			UPDATE	prezzi
			SET 	data_termine = var_nuova_data
			WHERE 	specie_di_pianta = var_specie AND 
					data_termine IS NULL;
			
			INSERT INTO prezzi (specie_di_pianta, data_adozione, valore) VALUES (var_specie, var_nuova_data, var_prezzo);
			SET var_aggiornamento_effettivo = 1;
		ELSE
			SET var_aggiornamento_effettivo = 0;
		END IF;

    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_residenza
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_residenza`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_residenza` (
	IN var_cliente 		VARCHAR(16), 
    IN var_residenza 	VARCHAR(64)
)
BEGIN
	UPDATE	clienti
    SET 	indirizzo_residenza = var_residenza
    WHERE 	codice = var_cliente;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_fatturazione
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_fatturazione`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_fatturazione` (
	IN var_cliente 		VARCHAR(16),
    IN var_fatturazione VARCHAR(64)
)
BEGIN
	UPDATE	clienti
    SET 	indirizzo_fatturazione = var_fatturazione
    WHERE 	codice = var_cliente;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_contatto_cliente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_contatto_cliente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_contatto_cliente` (
	IN var_cliente	VARCHAR(16),
	IN var_contatto	VARCHAR(256),
    IN var_tipo 	CHAR(16)
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45010' SET MESSAGE_TEXT = "Contact already inserted";
        ELSE
			RESIGNAL;  
        END IF;
    END;
    
	SET TRANSACTION ISOLATION LEVEL READ COMMITTED;
    START TRANSACTION;
    
		INSERT INTO contatti (`valore`, `tipo`) VALUES (var_contatto, var_tipo);
		INSERT INTO contatti_clienti (`contatto`, `cliente`) VALUES (var_contatto, var_cliente);
        
        SELECT 	contatti.valore	AS `Contact`,
				contatti.tipo	AS `Type`
		FROM	contatti INNER JOIN	contatti_clienti ON contatti.valore = contatti_clienti.contatto
        WHERE	cliente = var_cliente;
        
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure rimuovi_contatto_cliente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`rimuovi_contatto_cliente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `rimuovi_contatto_cliente` (
	IN var_cliente 	VARCHAR(16),
	IN var_contatto VARCHAR(256)
)
BEGIN
	DECLARE var_verifica_possesso INT;
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1451 THEN
			SIGNAL SQLSTATE '45012' SET MESSAGE_TEXT = "Cannot remove the contact because it is associated to an order";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	SELECT	COUNT(*)
    FROM 	contatti_clienti
    WHERE 	cliente = var_cliente AND
			contatto = var_contatto
	INTO	var_verifica_possesso;
    
    IF var_verifica_possesso = 0 THEN
		SIGNAL SQLSTATE '45009' SET MESSAGE_TEXT = "Selected customer does not own this contact";
    ELSE
		DELETE FROM contatti WHERE valore = var_contatto;
    END IF;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_contatto_preferito_cliente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_contatto_preferito_cliente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_contatto_preferito_cliente` (
	IN var_cliente 	VARCHAR(16),
    IN var_contatto VARCHAR(256)
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45013' SET MESSAGE_TEXT = "Cannot set this contact as favourite one because it is not owned yet";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	UPDATE	clienti
    SET 	contatto_preferito = var_contatto
    WHERE	codice = var_cliente;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_contatto_referente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_contatto_referente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_contatto_referente` (
	IN var_referente	CHAR(11),
	IN var_contatto 	VARCHAR(256),
    IN var_tipo 		CHAR(16)
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45010' SET MESSAGE_TEXT = "Contact already inserted";
        ELSE
			RESIGNAL;  
        END IF;
    END;
    
	-- Il livello di isolamento scelto e' REPEATABLE READ, 
    -- poiche' il report sui contatti posseduti dal referente
    -- a seguito dell'inserimento e' utilizzato prevalentemente
    -- come memo per il cliente per cui un eventuale inserimento 
    -- di un contatto in concorrenza e' tollerabile.
    START TRANSACTION;
    
		INSERT INTO contatti (`valore`, `tipo`) VALUES (var_contatto, var_tipo);
		INSERT INTO contatti_referenti (`contatto`, `referente`) VALUES (var_contatto, var_referente);
        
        SELECT 	contatti.valore	AS `Contact`,
				contatti.tipo	AS `Type`
		FROM	contatti INNER JOIN	contatti_referenti ON contatti.valore = contatti_referenti.contatto
        WHERE	referente = var_referente;        
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure rimuovi_contatto_referente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`rimuovi_contatto_referente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `rimuovi_contatto_referente` (
	IN var_referente 	CHAR(11),
	IN var_contatto 	VARCHAR(256)
)
BEGIN
	DECLARE var_verifica_possesso INT;
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1451 THEN
			SIGNAL SQLSTATE '45012' SET MESSAGE_TEXT = "Cannot remove the contact because it is associated to an order";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	SELECT	COUNT(*)
    FROM 	contatti_referenti
    WHERE 	referente = var_referente AND
			contatto = var_contatto
	INTO	var_verifica_possesso;
    
    IF var_verifica_possesso = 0 THEN
		SIGNAL SQLSTATE '45009' SET MESSAGE_TEXT = "Selected referent does not own this contact";
    ELSE
		DELETE FROM contatti WHERE valore = var_contatto;
    END IF;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure modifica_contatto_preferito_referente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`modifica_contatto_preferito_referente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `modifica_contatto_preferito_referente` (
	IN var_referente 	CHAR(11),
    IN var_contatto 	VARCHAR(256)
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45013' SET MESSAGE_TEXT = "Cannot set this contact as favourite one because it is not owned yet";
        ELSE
			RESIGNAL; 
        END IF;
    END;
    
	UPDATE referenti
    SET contatto_preferito = var_contatto
    WHERE rivendita = var_referente;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure rimuovi_specie_di_pianta
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`rimuovi_specie_di_pianta`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `rimuovi_specie_di_pianta` (
	IN var_codice INT,
    OUT var_eliminazione_effettiva INT
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1451 THEN
			SIGNAL SQLSTATE '45029' SET MESSAGE_TEXT = "Cannot remove the species because it is associated to an order";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	SELECT	COUNT(*)
    FROM	specie_di_piante
	WHERE	codice = var_codice
    INTO	var_eliminazione_effettiva;
	
    IF var_eliminazione_effettiva = 1 THEN
		DELETE FROM specie_di_piante WHERE codice = var_codice;
    END IF;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_colorazione
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_colorazione`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_colorazione` (
	IN var_specie_fiorita 	INT,
    IN var_colore 			VARCHAR(32)
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45014' SET MESSAGE_TEXT = "Species does not exists";
        ELSEIF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45030' SET MESSAGE_TEXT = "Coloring already specified for selected species";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	INSERT INTO colorazioni (`specie_fiorita`, `colore`) VALUES (var_specie_fiorita, var_colore);
    
	SELECT 	UPPER(colore)	AS `Coloring`
    FROM	colorazioni
    WHERE	specie_fiorita = var_specie_fiorita;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure rimuovi_colorazione
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`rimuovi_colorazione`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `rimuovi_colorazione` (
	IN var_specie_fiorita 	INT,
    IN var_colore			VARCHAR(32),
    OUT var_eliminazione_effettiva INT
)
BEGIN
	SELECT	COUNT(*)
    FROM	colorazioni
    WHERE	specie_fiorita = var_specie_fiorita AND
			colore = var_colore
	INTO	var_eliminazione_effettiva;
    
    IF var_eliminazione_effettiva = 1 THEN
		DELETE FROM colorazioni WHERE specie_fiorita = var_specie_fiorita AND colore = var_colore;
	END IF;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure inserisci_richiesta_fornitura
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`inserisci_richiesta_fornitura`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `inserisci_richiesta_fornitura` (
	IN var_fornitore 	INT,
    IN var_specie 		INT,
    IN var_quantita 	INT
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45025' SET MESSAGE_TEXT = "Either species or supplier does not exists";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	INSERT INTO richieste_di_forniture (`data`, `fornitore_scelto`, `specie_richiesta`, `quantita`) 
		VALUES (NOW(), var_fornitore, var_specie, var_quantita);
        
	SELECT		DATE(`richieste_di_forniture`.`data`)													AS 	`Date`,
				`fornitori`.`codice_fornitore`															AS	`Supplier_code`,
				`fornitori`.`nome`																		AS	`Supplier_name`,
				`richieste_di_forniture`.`quantita`														AS	`Quantity`
	
    FROM		`richieste_di_forniture` INNER JOIN `fornitori`	ON `richieste_di_forniture`.`fornitore_scelto` = `fornitori`.`codice_fornitore`
	
    WHERE		`richieste_di_forniture`.`specie_richiesta` = var_specie AND
				`richieste_di_forniture`.`pendente_si_no` = 1
	
    ORDER BY	`richieste_di_forniture`.`data` DESC;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure inserisci_fornitore
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`inserisci_fornitore`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `inserisci_fornitore` (
	IN 	var_codice_fiscale 		CHAR(16),
    IN 	var_nome 				VARCHAR(32),
    IN 	var_specie 				INT,
    IN 	var_indirizzo			VARCHAR(64),
    OUT var_codice_fornitore 	INT
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45035' SET MESSAGE_TEXT = "Supplier already inserted";
        ELSE
			RESIGNAL;  
        END IF;
    END;

	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE; 
    START TRANSACTION;
		INSERT INTO fornitori (`codice_fiscale`, `nome`) VALUES (var_codice_fiscale, var_nome);
		SELECT MAX(codice_fornitore) FROM fornitori INTO var_codice_fornitore;
		INSERT INTO disponibilita_forniture (`fornitore`, `specie_di_pianta`) VALUES (var_codice_fornitore, var_specie);
        INSERT INTO indirizzi_fornitori (`indirizzo`, `fornitore`) VALUES (var_indirizzo, var_codice_fornitore);
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_disponibilita_fornitura
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_disponibilita_fornitura`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_disponibilita_fornitura` (
	IN var_fornitore	INT,
    IN var_specie 		INT
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45025' SET MESSAGE_TEXT = "Either species or supplier does not exists";
        ELSEIF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45018' SET MESSAGE_TEXT = "Supply availability already inserted";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	INSERT INTO disponibilita_forniture (`fornitore`, `specie_di_pianta`) VALUES (var_fornitore, var_specie);
    
    SELECT 	codice		AS 	`Species_code`,
			nome_comune	AS 	`Species_common_name`,
            nome_latino	AS	`Species_latin_name`
	FROM	disponibilita_forniture	INNER JOIN specie_di_piante ON codice = specie_di_pianta
    WHERE	fornitore = var_fornitore;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure report_giacenza
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`report_giacenza`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `report_giacenza` (IN var_range INT)
BEGIN
	DECLARE var_codice_cursor INT;
	DECLARE done INT DEFAULT FALSE;
    DECLARE cur CURSOR FOR 
		SELECT		codice
        FROM 		specie_di_piante
        ORDER BY 	giacenza ASC
        LIMIT		var_range;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	START TRANSACTION;
    
        OPEN cur;
		read_loop: LOOP
			FETCH cur INTO var_codice_cursor;
			IF done THEN
				LEAVE read_loop;
			END IF;
            
			SELECT	codice										AS	`Species_code`,
					CONCAT(nome_comune, " (", nome_latino, ")")	AS 	`Species_name`,
					giacenza									AS 	`Stock`
			FROM	specie_di_piante
            WHERE	codice = var_codice_cursor;
            
			SELECT		codice_fornitore			AS	`Supplier_code`,
						nome						AS	`Supplier_name`,
						COUNT(fornitore_scelto)		AS 	`No_request_made`
			FROM 		fornitori 	INNER JOIN	disponibilita_forniture	ON codice_fornitore = fornitore
									LEFT JOIN	richieste_di_forniture 	ON fornitore_scelto = codice_fornitore
			WHERE		specie_di_pianta = var_codice_cursor AND
						(specie_richiesta = var_codice_cursor OR specie_richiesta IS NULL)
            GROUP BY	`Supplier_code`, `Supplier_name`
			ORDER BY	`No_request_made` DESC
			LIMIT		5;
			
        END LOOP;
		CLOSE cur;
        
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure report_pacchi
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`report_pacchi`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `report_pacchi` (IN var_ordine INT)
BEGIN
	DECLARE var_numero_pacco INT;

	DECLARE done INT DEFAULT FALSE;
    DECLARE cur CURSOR FOR
		SELECT	numero 
        FROM 	pacchi 
        WHERE 	ordine = var_ordine;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;    
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	DROP TEMPORARY TABLE IF EXISTS dettagli_pacchi;
    CREATE TEMPORARY TABLE dettagli_pacchi (
		`Pack_number` INT UNSIGNED,
		`Species_code` INT,
        `Latin_name` VARCHAR(64),  
        `Common_name` VARCHAR(64),  
        `Quantity` INT UNSIGNED
	);

	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
    START TRANSACTION;

        OPEN cur;
		read_loop: LOOP
			FETCH cur INTO var_numero_pacco;
			IF done THEN
				LEAVE read_loop;
			END IF;
            
            INSERT INTO dettagli_pacchi
				SELECT	pacco		AS	`Pack_number`,
						codice		AS	`Species_code`,
						nome_latino	AS	`Latin_name`,
						nome_comune	AS	`Common_name`,
						quantita	AS	`Quantity`
				FROM	specie_di_piante INNER JOIN	contenuto_pacchi ON codice = specie_di_pianta
				WHERE	ordine = var_ordine 		AND
						pacco = var_numero_pacco;
        END LOOP;
		CLOSE cur;
        
        SELECT 		* 
        FROM		dettagli_pacchi
        ORDER BY	`Pack_number`;
        
        DROP TEMPORARY TABLE dettagli_pacchi;
		
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_specie_a_pacco
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_specie_a_pacco`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_specie_a_pacco` (
	IN var_specie	INT,
    IN var_ordine 	INT,
	IN var_pacco 	INT,
    IN var_quantita	INT
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45026' SET MESSAGE_TEXT = "Either species or pack does not exists";
        ELSE
			RESIGNAL;  
        END IF;
    END;

	-- Il livello di isolamento scelto e' SERIALIZABLE, 
    -- per via della funzione aggregata sum utilizzata nel trigger BI
	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
    START TRANSACTION;
		INSERT INTO contenuto_pacchi(`ordine`, `pacco`, `specie_di_pianta`, `quantita`) VALUES (var_ordine, var_pacco, var_specie, var_quantita);
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure report_specie
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`report_specie`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `report_specie` (IN var_codice INT)
BEGIN
    DECLARE var_nome VARCHAR(131);
    DECLARE var_mese INT DEFAULT 1;
    DECLARE var_mese_string VARCHAR(16);
    DECLARE var_vendite INT;
    DECLARE var_verifica_esistenza INT;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	DROP TEMPORARY TABLE IF EXISTS `report_mensile`;
    CREATE TEMPORARY TABLE `report_mensile` (
		`Month` VARCHAR(16),
        `Sales` INT
    );
    
    START TRANSACTION;
    
		SELECT COUNT(*)
        FROM specie_di_piante 
        WHERE codice = var_codice
		INTO var_verifica_esistenza;
        
        IF var_verifica_esistenza <> 1 THEN
			SIGNAL SQLSTATE '45014' SET MESSAGE_TEXT = "Species does not exists";
		END IF;
        
		SELECT	codice		AS `Species_code`,
				nome_comune AS `Common_name`,
				nome_latino	AS `Latin_name`
		FROM	specie_di_piante
		WHERE	codice = var_codice;
			
		for_loop: LOOP
		
			IF var_mese = 13 THEN
				LEAVE for_loop;
			END IF;
		
			SELECT	SUM(`appartenenza_ordini`.`quantita`)
			FROM	`appartenenza_ordini`	INNER JOIN `ordini` ON `appartenenza_ordini`.`ordine` = `ordini`.`id`
			WHERE	`aperto_or_finalizzato` = 0		AND
					YEAR(`data`) = YEAR(CURDATE()) 	AND
					MONTH(`data`) = var_mese		AND
                    `specie_di_pianta` = var_codice
			INTO	var_vendite;
            
            IF var_vendite IS NULL THEN SET var_vendite = 0; END IF;
            
            CASE var_mese
				WHEN 1 THEN SET var_mese_string = "January";
                WHEN 2 THEN SET var_mese_string = "February";
                WHEN 3 THEN SET var_mese_string = "March";
                WHEN 4 THEN SET var_mese_string = "April";
                WHEN 5 THEN SET var_mese_string = "May";
                WHEN 6 THEN SET var_mese_string = "June";
                WHEN 7 THEN SET var_mese_string = "July";
                WHEN 8 THEN SET var_mese_string = "August";
                WHEN 9 THEN SET var_mese_string = "September";
                WHEN 10 THEN SET var_mese_string = "October";
                WHEN 11 THEN SET var_mese_string = "November";
                WHEN 12 THEN SET var_mese_string = "December";
                ELSE SET var_mese_string = "####"; 
            END CASE;
			
			INSERT INTO `report_mensile` (`Month`, `Sales`)
				VALUES (var_mese_string, var_vendite);
				
			SET var_mese = var_mese + 1;
			
		END LOOP;
        
        SELECT * FROM `report_mensile`; 
		
		DROP TEMPORARY TABLE `report_mensile`;   

	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_ordini_cliente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_ordini_cliente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_ordini_cliente` (
	IN var_cliente VARCHAR(16),
    IN var_status TINYINT
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	SET TRANSACTION ISOLATION LEVEL READ COMMITTED; 
    START TRANSACTION;
    
		IF var_status = 1 THEN
			SELECT		`id`					AS	`Order`,
						`indirizzo_spedizione`	AS	`Shipping_address`,
						DATE(`data`)			AS	`Date`
			FROM		`ordini`
			WHERE		`cliente` = var_cliente	AND
						`aperto_or_finalizzato` = 1
			ORDER BY	`data` DESC;
		ELSE
			SELECT		`id`												AS	`Order`,
						`indirizzo_spedizione`								AS	`Shipping address`,
						DATE(`data`)										AS	`Date`,
						CASE
							WHEN `aperto_or_finalizzato` = 1 THEN "Open"
							ELSE "Finalized"
						END 												AS `Status`
			FROM		`ordini`
			WHERE		`cliente` = var_cliente
			ORDER BY	`data` DESC;
		END IF;
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_contatti_cliente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_contatti_cliente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_contatti_cliente` (IN var_cliente VARCHAR(16))
BEGIN
	SELECT 	contatti.valore	AS `Contact`,
			contatti.tipo	AS `Type`
	FROM	contatti INNER JOIN	contatti_clienti ON contatti.valore = contatti_clienti.contatto
	WHERE	cliente = var_cliente;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_contatti_referente
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_contatti_referente`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_contatti_referente` (IN var_referente VARCHAR(16))
BEGIN
	SELECT 	contatti.valore	AS `Contact`,
			contatti.tipo	AS `Type`
	FROM	contatti INNER JOIN	contatti_referenti ON contatti.valore = contatti_referenti.contatto
	WHERE	referente = var_referente;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_colorazioni
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_colorazioni`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_colorazioni` (IN var_specie_fiorita INT)
BEGIN
	SELECT	UPPER(colore)	AS `Coloring`
	FROM	colorazioni
	WHERE	specie_fiorita = var_specie_fiorita;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_dettagli_specie
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_dettagli_specie`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_dettagli_specie` (
	IN var_nome_comune VARCHAR(64),
	IN var_solo_fiorite TINYINT
)
BEGIN
    DECLARE var_ricerca VARCHAR(66);
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	-- Il livello di isolamento scelto e' READ COMMITED perche'
    -- l'unico interesse in questa situazione e' quello di evitare
    -- dirty reads (non si effettuano letture ulteriori ne' vengono
    -- riutilizzati i dati letti).
	SET TRANSACTION ISOLATION LEVEL READ COMMITTED; 
    START TRANSACTION;
		
		IF var_nome_comune IS NOT NULL THEN
			SET var_ricerca = CONCAT("%", UPPER(var_nome_comune), "%");
		ELSE
			SET var_ricerca = "%";
		END IF;
		
        IF var_solo_fiorite = 0 THEN
			SELECT		codice											AS	`Code`,
						nome_latino 									AS	`Latin_name`,
						nome_comune 									AS	`Common_name`,
						CASE
							WHEN interni_or_esterni = 1 THEN 'Indoor'
							ELSE 'Outdoor'
						END 											AS `Indoor/Outdoor`,
						CASE
							WHEN verde_or_fiorita = 1 THEN 'Green'
							ELSE 'Flowery'
						END 											AS `Green/Flowery`,                
						CASE
							WHEN esotica_si_no = 1 THEN 'Yes'
							ELSE 'No'
						END 											AS `Exotic`, 
						valore											AS `Price`,
                        giacenza										AS `Stock`
			FROM		specie_di_piante INNER JOIN prezzi ON specie_di_piante.codice = prezzi.specie_di_pianta
			WHERE		UPPER(nome_comune) LIKE var_ricerca AND
						data_termine IS NULL
			ORDER BY	nome_comune ASC;
		ELSE
			SELECT		codice											AS `Code`,
						nome_latino 									AS `Latin name`,
						nome_comune AS	`Common name`,
						CASE
							WHEN interni_or_esterni = 1 THEN 'Indoor'
							ELSE 'Outdoor'
						END 											AS `Indoor/Outdoor`,
						CASE
							WHEN verde_or_fiorita = 1 THEN 'Green'
							ELSE 'Flowery'
						END 											AS `Green/Flowery`,                
						CASE
							WHEN esotica_si_no = 1 THEN 'Yes'
							ELSE 'No'
						END 											AS `Exotic`, 
						valore											AS `Price`,
                        giacenza										AS `Stock`
			FROM		specie_di_piante INNER JOIN prezzi ON specie_di_piante.codice = prezzi.specie_di_pianta
			WHERE		UPPER(nome_comune) LIKE var_ricerca AND
						data_termine IS NULL AND
                        verde_or_fiorita = 0 
			ORDER BY	nome_comune ASC;
		END IF;
	
    COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_fornitori
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_fornitori`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_fornitori` (IN var_nome VARCHAR(32))
BEGIN
    DECLARE var_ricerca VARCHAR(34);
	
	IF var_nome IS NOT NULL THEN
		SET var_ricerca = CONCAT("%", var_nome, "%");
	ELSE
		SET var_ricerca = "%";
	END IF;
	
	SELECT		codice_fornitore	AS	`Supplier_code`,
				codice_fiscale		AS	`Fiscal_code`,
				nome				AS	`Name`
	FROM		fornitori 
	WHERE		nome LIKE var_ricerca 
	ORDER BY	codice_fornitore DESC;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_specie_disponibili
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_specie_disponibili`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_specie_disponibili` (IN var_fornitore INT)
BEGIN
	SELECT		codice		AS	`Code`,
				nome_latino AS	`Latin_name`,
				nome_comune AS	`Common_name`
	FROM		specie_di_piante INNER JOIN disponibilita_forniture ON codice = specie_di_pianta
	WHERE		fornitore = var_fornitore
	ORDER BY	nome_comune ASC;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_stato_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_stato_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_stato_ordine` (IN var_ordine INT)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
    START TRANSACTION;
		
		SELECT	`ordini`.`id`														AS	`Order_ID`,											
				DATE(`ordini`.`data`)												AS	`Date`,
				`ordini`.`indirizzo_spedizione`										AS	`Shipping_address`,
				`ordini`.`contatto`													AS	`Chosen_contact`,
				CASE
					WHEN verifica_completamento(`id`) 	THEN "Completed"
					WHEN aperto_or_finalizzato = 1 		THEN "Already open"
					ELSE "Not yet completed"
				END 																AS `Status`,
				CASE 
				WHEN `clienti`.`privato_or_rivendita` = 1 THEN "Not expected"
					ELSE concat(`referenti`.`nome`, " ", `referenti`.`cognome`)
				END 																AS `Referent`
		FROM	`ordini` 	INNER JOIN 	`clienti`	ON `ordini`.`cliente` = `clienti`.`codice`
							LEFT JOIN	`referenti`	ON `clienti`.`codice` = `referenti`.`rivendita`	
		WHERE	`id` = var_ordine;
    
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_piante_rimanenti_da_impacchettare
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_piante_rimanenti_da_impacchettare`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_piante_rimanenti_da_impacchettare` (IN var_ordine INT)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	-- Il livello di isolamento scelto e' READ COMMITED perche'
    -- l'unico interesse in questa situazione e' quello di evitare
    -- dirty reads (non si effettuano letture ulteriori ne' vengono
    -- riutilizzati i dati letti).
	SET TRANSACTION ISOLATION LEVEL READ COMMITTED; 
    START TRANSACTION;
    
		SELECT		specie_di_piante.codice																AS	`Species_code`,
					CONCAT(specie_di_piante.nome_comune, " (", specie_di_piante.nome_latino, + ")")		AS	`Species_name`,
					appartenenza_ordini.quantita														AS 	`Quantity`,
                    CASE
						WHEN SUM(contenuto_pacchi.quantita) IS NULL THEN 0
                        ELSE SUM(contenuto_pacchi.quantita)
					END																					AS	`Already_processed`  
					
		FROM		specie_di_piante 	LEFT JOIN	contenuto_pacchi	
										ON 			contenuto_pacchi.specie_di_pianta = specie_di_piante.codice
										
										INNER JOIN	appartenenza_ordini	
										ON 			appartenenza_ordini.specie_di_pianta = specie_di_piante.codice

		WHERE		appartenenza_ordini.ordine = var_ordine AND
					(contenuto_pacchi.ordine = var_ordine OR contenuto_pacchi.ordine IS NULL)
		GROUP BY	`Species_code`, `Species_name`, `Quantity`;
        
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure visualizza_specie_appartenenti_ad_ordine
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`visualizza_specie_appartenenti_ad_ordine`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `visualizza_specie_appartenenti_ad_ordine` (IN var_ordine INT)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;  
        RESIGNAL;  
    END;
    
	-- Il livello di isolamento scelto e' READ COMMITED perche'
    -- l'unico interesse in questa situazione e' quello di evitare
    -- dirty reads (non si effettuano letture ulteriori ne' vengono
    -- riutilizzati i dati letti).
	SET TRANSACTION ISOLATION LEVEL READ COMMITTED; 
    START TRANSACTION;
    
		SELECT		`codice`											AS	`Species_code`,
					CONCAT(`nome_comune`, " (", `nome_latino`, + ")")	AS	`Species_name`,
                    `quantita`											AS 	`Quantity`
		FROM		`appartenenza_ordini`	INNER JOIN	`specie_di_piante`	ON `specie_di_pianta` = `codice`
        WHERE		`ordine` = var_ordine;
        
	COMMIT;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure aggiungi_indirizzo_fornitore
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`aggiungi_indirizzo_fornitore`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `aggiungi_indirizzo_fornitore` (
	IN var_fornitore	INT,
    IN var_indirizzo	VARCHAR(64)
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45032' SET MESSAGE_TEXT = "Supplier does not exists";
        ELSEIF var_errno = 1062 THEN
			SIGNAL SQLSTATE '45031' SET MESSAGE_TEXT = "Address already inserted";
        ELSE
			RESIGNAL; 
        END IF;
    END;

	INSERT INTO indirizzi_fornitori (`indirizzo`, `fornitore`) VALUES (var_indirizzo, var_fornitore);
    
    SELECT 	UPPER(indirizzo)	AS 	`Address`
	FROM	indirizzi_fornitori
    WHERE	fornitore = var_fornitore;
END$$

DELIMITER ;

-- -----------------------------------------------------
-- procedure crea_pacco
-- -----------------------------------------------------

USE `verdesrl`;
DROP procedure IF EXISTS `verdesrl`.`crea_pacco`;

DELIMITER $$
USE `verdesrl`$$
CREATE PROCEDURE `crea_pacco` (
	IN	var_ordine		INT,
    IN 	var_specie 		INT,
    IN 	var_quantita 	INT,
    OUT var_numero 		INT
)
BEGIN
	DECLARE var_errno INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        
        GET DIAGNOSTICS CONDITION 1 var_errno = MYSQL_ERRNO;
        
		IF var_errno = 1452 THEN
			SIGNAL SQLSTATE '45027' SET MESSAGE_TEXT = "Either species or order does not exists";
        ELSE
			RESIGNAL;  
        END IF;
    END;
    
    SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
    START TRANSACTION;
        
		SELECT	MAX(numero) + 1
		FROM	pacchi
		WHERE	ordine = var_ordine
		INTO	var_numero;
        
        IF var_numero IS NULL THEN 
			SET var_numero = 1;
		END IF;
    
		INSERT INTO pacchi (`ordine`, `numero`) VALUES (var_ordine, var_numero);
		INSERT INTO contenuto_pacchi(`ordine`, `pacco`, `specie_di_pianta`, `quantita`) VALUES (var_ordine, var_numero, var_specie, var_quantita);
    
    COMMIT;
END$$

DELIMITER ;
SET SQL_MODE = '';
DROP USER IF EXISTS non_registrato;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'non_registrato' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`login` TO 'non_registrato';
GRANT EXECUTE ON procedure `verdesrl`.`registra_privato` TO 'non_registrato';
GRANT EXECUTE ON procedure `verdesrl`.`registra_rivendita` TO 'non_registrato';
SET SQL_MODE = '';
DROP USER IF EXISTS cliente_privato;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'cliente_privato' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`modifica_password` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`crea_ordine` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_specie_da_ordine` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_ordine` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_specie_ad_ordine_esistente` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`finalizza_ordine` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`report_ordine` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_residenza` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_fatturazione` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_contatto_cliente` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_contatto_cliente` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_contatto_preferito_cliente` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_ordini_cliente` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_contatti_cliente` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_dettagli_specie` TO 'cliente_privato';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_specie_appartenenti_ad_ordine` TO 'cliente_privato';
SET SQL_MODE = '';
DROP USER IF EXISTS cliente_rivendita;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'cliente_rivendita' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`modifica_password` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`crea_ordine` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_specie_da_ordine` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_ordine` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_specie_ad_ordine_esistente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`finalizza_ordine` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`report_ordine` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_residenza` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_fatturazione` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_contatto_cliente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_contatto_cliente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_contatto_preferito_cliente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_contatto_referente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_contatto_referente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_contatto_preferito_referente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_ordini_cliente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_contatti_cliente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_contatti_referente` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_dettagli_specie` TO 'cliente_rivendita';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_specie_appartenenti_ad_ordine` TO 'cliente_rivendita';
SET SQL_MODE = '';
DROP USER IF EXISTS addetto_diparimento_magazzino;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'addetto_diparimento_magazzino' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`modifica_password` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`inserisci_richiesta_fornitura` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`inserisci_fornitore` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_disponibilita_fornitura` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`report_giacenza` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_dettagli_specie` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_fornitori` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_specie_disponibili` TO 'addetto_diparimento_magazzino';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_indirizzo_fornitore` TO 'addetto_diparimento_magazzino';
SET SQL_MODE = '';
DROP USER IF EXISTS operatore_pacchi;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'operatore_pacchi' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`modifica_password` TO 'operatore_pacchi';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_specie_a_pacco` TO 'operatore_pacchi';
GRANT EXECUTE ON procedure `verdesrl`.`report_pacchi` TO 'operatore_pacchi';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_stato_ordine` TO 'operatore_pacchi';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_piante_rimanenti_da_impacchettare` TO 'operatore_pacchi';
GRANT EXECUTE ON procedure `verdesrl`.`crea_pacco` TO 'operatore_pacchi';
SET SQL_MODE = '';
DROP USER IF EXISTS manager;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'manager' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`modifica_password` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`modifica_prezzo` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`inserisci_nuova_specie` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_specie_di_pianta` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`aggiungi_colorazione` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`rimuovi_colorazione` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`report_specie` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_colorazioni` TO 'manager';
GRANT EXECUTE ON procedure `verdesrl`.`visualizza_dettagli_specie` TO 'manager';
SET SQL_MODE = '';
DROP USER IF EXISTS capo_personale;
SET SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';
CREATE USER 'capo_personale' IDENTIFIED BY 'verdesrl';

GRANT EXECUTE ON procedure `verdesrl`.`modifica_password` TO 'capo_personale';
GRANT EXECUTE ON procedure `verdesrl`.`crea_utenza_dipendente` TO 'capo_personale';

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

-- -----------------------------------------------------
-- Data for table `verdesrl`.`utenti`
-- -----------------------------------------------------
START TRANSACTION;
USE `verdesrl`;
INSERT INTO `verdesrl`.`utenti` (`username`, `password`, `ruolo`, `uuid`) VALUES ('admin', '7603e55f21a11a3c9e71a00110dd97cc1552d7954bf90685eaa91c98bbfac26e24d4e9af41a122a212ccd423a7e305820cb57c7cfaa88409c78ff832859532b3', 'COS', '058fa777-84c0-11ea-8c72-d0039b002ee5');

COMMIT;

USE `verdesrl`;

DELIMITER $$

USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`utenti_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`utenti_BEFORE_INSERT` BEFORE INSERT ON `utenti` FOR EACH ROW
BEGIN
	IF INSTR(NEW.`username`, " ") > 0 THEN
		SIGNAL SQLSTATE '45004' SET MESSAGE_TEXT = "Username must not contains spaces";
	END IF; 
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`specie_di_piante_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`specie_di_piante_BEFORE_INSERT` BEFORE INSERT ON `specie_di_piante` FOR EACH ROW
BEGIN
	IF NEW.interni_or_esterni <> 1  AND NEW.interni_or_esterni <> 0 THEN
		SIGNAL SQLSTATE '45022' SET MESSAGE_TEXT = "Types of species allowed: 1 -> Indoor ~ 0 -> Outdoor";
    END IF;	
	IF NEW.verde_or_fiorita <> 1  AND NEW.verde_or_fiorita <> 0 THEN
		SIGNAL SQLSTATE '45022' SET MESSAGE_TEXT = "Types of species allowed: 1 -> Green ~ 0 -> Flowery";
    END IF;	
	IF NEW.esotica_si_no <> 1  AND NEW.esotica_si_no <> 0 THEN
		SIGNAL SQLSTATE '45022' SET MESSAGE_TEXT = "Types of species allowed: 1 -> Exotic ~ 0 -> Not exotic";
    END IF;	    
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`clienti_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`clienti_BEFORE_INSERT` BEFORE INSERT ON `clienti` FOR EACH ROW
BEGIN
	IF NEW.codice REGEXP "[A-Za-z]{6}[0-9]{2}[A-Za-z]{1}[0-9]{2}[A-Za-z]{1}[0-9]{3}[A-Za-z]{1}$" THEN
		IF NEW.privato_or_rivendita <> 1 THEN
			SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = "Mismatch between code type and customer type";
		END IF;
	ELSEIF NEW.codice REGEXP "[0-9]{11}$" THEN
		IF NEW.privato_or_rivendita <> 0 THEN
			SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = "Mismatch between code type and customer type";
		END IF;
	ELSE
		SIGNAL SQLSTATE '45001' SET MESSAGE_TEXT = "Wrong code inserted";
	END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`clienti_BEFORE_UPDATE` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`clienti_BEFORE_UPDATE` BEFORE UPDATE ON `clienti` FOR EACH ROW
BEGIN
	DECLARE var_proprietario VARCHAR(16);
	
    IF NEW.contatto_preferito IS NOT NULL AND NEW.contatto_preferito <> OLD.contatto_preferito THEN
		SELECT	cliente
        FROM	contatti_clienti
        WHERE	contatto = NEW.contatto_preferito
        INTO	var_proprietario;
        
        IF NEW.codice <> var_proprietario THEN
			SIGNAL SQLSTATE '45009' SET MESSAGE_TEXT = "Selected customer does not own this contact";
		END IF;
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`referenti_BEFORE_UPDATE` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`referenti_BEFORE_UPDATE` BEFORE UPDATE ON `referenti` FOR EACH ROW
BEGIN
	DECLARE var_proprietario VARCHAR(16);
	
    IF NEW.contatto_preferito IS NOT NULL AND NEW.contatto_preferito <> OLD.contatto_preferito THEN
		SELECT	referente
        FROM	contatti_referenti
        WHERE	contatto = NEW.contatto_preferito
        INTO var_proprietario;
        
        IF NEW.rivendita <> var_proprietario THEN
			SIGNAL SQLSTATE '45009' SET MESSAGE_TEXT = "Selected referent does not own this contact";
		END IF;
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`pacchi_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`pacchi_BEFORE_INSERT` BEFORE INSERT ON `pacchi` FOR EACH ROW
BEGIN
	DECLARE var_stato TINYINT;
    
    SELECT	aperto_or_finalizzato
    FROM	ordini
    WHERE	id = NEW.ordine
    INTO 	var_stato;
    
    IF var_stato = 1 THEN
		SIGNAL SQLSTATE '45034' SET MESSAGE_TEXT = "Order not yet finalized";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`fornitori_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`fornitori_BEFORE_INSERT` BEFORE INSERT ON `fornitori` FOR EACH ROW
BEGIN
	IF NEW.codice_fiscale REGEXP "/^[A-Za-z]{6}[0-9]{2}[A-Za-z]{1}[0-9]{2}[A-Za-z]{1}[0-9]{3}[A-Za-z]{1}$/" THEN
		SIGNAL SQLSTATE '45023' SET MESSAGE_TEXT = "Fiscal code expression not valid";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`fornitori_BEFORE_INSERT_1` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`fornitori_BEFORE_INSERT_1` BEFORE INSERT ON `fornitori` FOR EACH ROW
BEGIN
	SET NEW.codice_fiscale = UPPER(NEW.codice_fiscale);
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`richieste_di_forniture_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`richieste_di_forniture_BEFORE_INSERT` BEFORE INSERT ON `richieste_di_forniture` FOR EACH ROW
BEGIN
	DECLARE var_verifica_disponibilita INT;
    
    SELECT	COUNT(*)
    FROM 	disponibilita_forniture
    WHERE 	fornitore = NEW.fornitore_scelto AND
			specie_di_pianta = NEW.specie_richiesta
	INTO	var_verifica_disponibilita;
    
    IF var_verifica_disponibilita = 0 THEN
		SIGNAL SQLSTATE '45024' SET MESSAGE_TEXT = "Request species not available";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`colorazioni_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`colorazioni_BEFORE_INSERT` BEFORE INSERT ON `colorazioni` FOR EACH ROW
BEGIN
	DECLARE var_verde_or_fiorita TINYINT;
    
    SELECT	verde_or_fiorita
    FROM	specie_di_piante
    WHERE	codice = NEW.specie_fiorita
    INTO	var_verde_or_fiorita;
    
    IF var_verde_or_fiorita = 1 THEN
		SIGNAL SQLSTATE '45019' SET MESSAGE_TEXT = "Green species cannot be colorful";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`contenuto_pacchi_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`contenuto_pacchi_BEFORE_INSERT` BEFORE INSERT ON `contenuto_pacchi` FOR EACH ROW
BEGIN
	DECLARE var_verifica_appartenenza INT;
    
	SELECT 	COUNT(*)
    FROM 	appartenenza_ordini
    WHERE 	ordine = NEW.ordine AND
			specie_di_pianta = NEW.specie_di_pianta
	INTO	var_verifica_appartenenza;
    
    IF var_verifica_appartenenza = 0 THEN
		SIGNAL SQLSTATE '45020' SET MESSAGE_TEXT = "Species does not belong to the order";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`contenuto_pacchi_BEFORE_INSERT_1` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`contenuto_pacchi_BEFORE_INSERT_1` BEFORE INSERT ON `contenuto_pacchi` FOR EACH ROW
BEGIN
	DECLARE var_quantita_ordine INT;
    DECLARE var_gia_impacchettate INT;
    
	SELECT	quantita
	FROM 	appartenenza_ordini
	WHERE 	specie_di_pianta = NEW.specie_di_pianta AND
			ordine = NEW.ordine
	INTO 	var_quantita_ordine;
	
	SELECT	SUM(quantita)
	FROM 	contenuto_pacchi
	WHERE	ordine = NEW.ordine AND
			specie_di_pianta = NEW.specie_di_pianta
	INTO 	var_gia_impacchettate;
    
    IF var_gia_impacchettate IS NULL THEN
		SET var_gia_impacchettate = 0;
	END IF;

	IF NEW.quantita > var_quantita_ordine - var_gia_impacchettate THEN
		SIGNAL SQLSTATE '45015' SET MESSAGE_TEXT = "Selected quantity greater than that not processed yet";
	END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_BEFORE_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_BEFORE_INSERT` BEFORE INSERT ON `appartenenza_ordini` FOR EACH ROW
BEGIN
	DECLARE var_giacenza INT UNSIGNED;
    
    SELECT	giacenza 
    FROM	specie_di_piante
    WHERE	codice = NEW.specie_di_pianta 
    INTO	var_giacenza;
    
    IF NEW.quantita > var_giacenza THEN
		SIGNAL SQLSTATE '45006' SET MESSAGE_TEXT = "Selected quantity is higher than current stock";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_BEFORE_INSERT_1` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_BEFORE_INSERT_1` BEFORE INSERT ON `appartenenza_ordini` FOR EACH ROW
BEGIN
    DECLARE var_stato TINYINT;
    
    SELECT	aperto_or_finalizzato
    FROM 	ordini
    WHERE 	id = NEW.ordine 
    INTO 	var_stato;
    
    IF var_stato = 0 THEN
		SIGNAL SQLSTATE '45007' SET MESSAGE_TEXT = "It is not possible to modify an order already closed";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_AFTER_INSERT` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_AFTER_INSERT` AFTER INSERT ON `appartenenza_ordini` FOR EACH ROW
BEGIN
	UPDATE 	specie_di_piante 
    SET 	giacenza = giacenza - NEW.quantita 
    WHERE 	codice = NEW.specie_di_pianta;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_BEFORE_UPDATE` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_BEFORE_UPDATE` BEFORE UPDATE ON `appartenenza_ordini` FOR EACH ROW
BEGIN
	DECLARE var_giacenza INT UNSIGNED;
    
    SELECT	giacenza
    FROM 	specie_di_piante
    WHERE 	codice = NEW.specie_di_pianta 
    INTO	var_giacenza;
    
    IF CAST(NEW.quantita AS SIGNED) - CAST(OLD.quantita AS SIGNED) > var_giacenza THEN
		SIGNAL SQLSTATE '45006' SET MESSAGE_TEXT = "Selected quantity is higher than current stock";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_BEFORE_UPDATE_1` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_BEFORE_UPDATE_1` BEFORE UPDATE ON `appartenenza_ordini` FOR EACH ROW
BEGIN
    DECLARE var_stato TINYINT;
    
    SELECT 	aperto_or_finalizzato
    FROM 	ordini
    WHERE 	id = NEW.ordine 
    INTO 	var_stato;
    
    IF var_stato = 0 THEN
		SIGNAL SQLSTATE '45007' SET MESSAGE_TEXT = "It is not possible to modify an order already closed";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_AFTER_UPDATE` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_AFTER_UPDATE` AFTER UPDATE ON `appartenenza_ordini` FOR EACH ROW
BEGIN
	/* l'if else e' necessario per via del fatto che giacenza
     * e' definita come unsigned int
     */
	IF NEW.quantita > OLD.quantita THEN
		UPDATE	specie_di_piante 
		SET giacenza = giacenza - (NEW.quantita - OLD.quantita)
		WHERE codice = NEW.specie_di_pianta;
    ELSE
		UPDATE	specie_di_piante 
		SET giacenza = giacenza + (OLD.quantita - NEW.quantita)
		WHERE codice = NEW.specie_di_pianta;
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_BEFORE_DELETE` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_BEFORE_DELETE` BEFORE DELETE ON `appartenenza_ordini` FOR EACH ROW
BEGIN
    DECLARE var_stato TINYINT;
    
    SELECT	aperto_or_finalizzato
    FROM 	ordini
    WHERE 	id = OLD.ordine 
    INTO 	var_stato;
    
    IF var_stato = 0 THEN
		SIGNAL SQLSTATE '45007' SET MESSAGE_TEXT = "It is not possible to modify an order already closed";
    END IF;
END$$


USE `verdesrl`$$
DROP TRIGGER IF EXISTS `verdesrl`.`appartenenza_ordini_AFTER_DELETE` $$
USE `verdesrl`$$
CREATE DEFINER = CURRENT_USER TRIGGER `verdesrl`.`appartenenza_ordini_AFTER_DELETE` AFTER DELETE ON `appartenenza_ordini` FOR EACH ROW
BEGIN
    DECLARE var_stato TINYINT;
    
    SELECT	aperto_or_finalizzato
    FROM	ordini
    WHERE	id = OLD.ordine 
    INTO 	var_stato;

	IF var_stato = 1 THEN
		UPDATE	specie_di_piante 
		SET		giacenza = giacenza + OLD.quantita 
		WHERE 	codice = OLD.specie_di_pianta;
    END IF;
END$$


DELIMITER ;
-- begin attached script 'script'
SET GLOBAL event_scheduler = ON;

DELIMITER $$

CREATE EVENT IF NOT EXISTS pulizia_storico_dati
ON SCHEDULE EVERY 1 YEAR
STARTS "2021-01-01 00:00:00"
ON COMPLETION PRESERVE
COMMENT "Eliminazione dati relativi ad ordini prezzi e richieste di fornitura memorizzati da piu' di due anni "
DO BEGIN
	DELETE FROM `verdesrl`.`ordini`
	WHERE `data` < (NOW() - INTERVAL 2 YEAR);
    
	DELETE FROM `verdesrl`.`richieste_di_forniture`
	WHERE `data` < (NOW() - INTERVAL 2 YEAR);
    
	DELETE FROM `verdesrl`.`prezzi`
	WHERE `data_termine` < (NOW() - INTERVAL 2 YEAR);
END $$

DELIMITER ;

DELIMITER $$

CREATE EVENT IF NOT EXISTS aggiornamento_giacenza
ON SCHEDULE EVERY 1 WEEK
STARTS "2020-04-21 00:00:00" -- supponendo lo scarico della merce avvenga il lunedì l'evento viene eseguito ogni martedì a mezzanotte.
ON COMPLETION PRESERVE
COMMENT "Aggiornamento giacenza al fronte di richieste di forniture"
DO BEGIN
	   
	DECLARE var_specie_c INT;
    DECLARE var_quantita_c INT;
    DECLARE var_data_agg DATETIME;
	DECLARE done INT DEFAULT FALSE;
    DECLARE cur CURSOR FOR 
		SELECT		`specie_richiesta`, `quantita`
        FROM 		`richieste_di_forniture`
        WHERE		`pendente_si_no` = 1;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = TRUE;

	DROP TEMPORARY TABLE IF EXISTS da_aggiornare;
    CREATE TEMPORARY TABLE da_aggiornare (
		`specie_di_pianta`	INT,
        `quantita_richiesta`	INT UNSIGNED
    );
    
    SET var_data_agg = now();
    
	OPEN cur;
	read_loop: LOOP
		FETCH cur INTO var_specie_c, var_quantita_c;
		IF done THEN
			LEAVE read_loop;
		END IF;
		
		UPDATE	`specie_di_piante`
        SET 	`giacenza` = `giacenza` + var_quantita_c
        WHERE	`codice` = var_specie_c;
	END LOOP;
	CLOSE cur;
    DROP TEMPORARY TABLE da_aggiornare;  
    
    UPDATE	`richieste_di_forniture`
    SET		`pendente_si_no` = 0
    WHERE	`data` <= var_data_agg;
    
END $$

DELIMITER ;
-- end attached script 'script'
-- begin attached script 'script1'
DELIMITER $$
CREATE FUNCTION `verifica_proprietario`(id_ordine INT, codice_cliente VARCHAR(16))
RETURNS BOOL
DETERMINISTIC
BEGIN

	DECLARE var_cliente_ordine VARCHAR(16);
    
    SELECT	cliente
    FROM	ordini
    WHERE	id = id_ordine
    INTO	var_cliente_ordine;
    
    RETURN (var_cliente_ordine = codice_cliente);

END $$

DELIMITER ;
-- end attached script 'script1'
-- begin attached script 'script2'
DELIMITER $$
CREATE FUNCTION `verifica_password`(var_password VARCHAR(128))
RETURNS TINYINT
DETERMINISTIC
BEGIN

	IF LENGTH(var_password) < 8 THEN
		RETURN 1;
    END IF;
    
	IF INSTR(var_password, " ") > 0 THEN
		RETURN 2;
	END IF;
    
    RETURN 0;
    
END $$

DELIMITER ;
-- end attached script 'script2'
-- begin attached script 'script3'
DELIMITER $$
CREATE FUNCTION `verifica_completamento`(var_ordine INT)
RETURNS BOOL
DETERMINISTIC
BEGIN
	DECLARE var_quantita_in_pacchi INT UNSIGNED;
    DECLARE var_quantita_in_ordine INT UNSIGNED;
    
    SELECT	SUM(quantita)
    FROM	appartenenza_ordini
    WHERE	ordine = var_ordine
	INTO	var_quantita_in_ordine;
    
    SELECT	SUM(quantita)
    FROM	contenuto_pacchi
    WHERE	ordine = var_ordine
    INTO	var_quantita_in_pacchi;
    
    RETURN var_quantita_in_pacchi = var_quantita_in_ordine;
END $$

DELIMITER ;
-- end attached script 'script3'
