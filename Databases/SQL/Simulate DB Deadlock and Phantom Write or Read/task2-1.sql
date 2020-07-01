SET SERVER OUTPUT ON
SET ECHO ON
CREATE OR REPLACE PROCEDURE Bonuses AS
BEGIN
	For Currlist IN (Select pnumber
					 FROM Position
					 WHERE bonus != NULL OR bonus != 0.0)
	LOOP
	UPDATE POSITION
	Set bonus = bonus *0.05
	WHERE pnumber = Currlist.pnumber;
	END LOOP;
	
	For Nextlist IN (Select pnumber
					 FROM Position
					 WHERE bonus = NULL OR bonus = 0.0)
	LOOP
	UPDATE Position
	Set bonus = 0.0
	WHERE pnumber = Nextlist.pnumber;
	END LOOP; 
END;
/
show errors