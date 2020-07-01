SET SERVER OUTPUT ON
SET ECHO ON
CREATE OR REPLACE PROCEDURE Extras AS
BEGIN
	For Currlist IN (Select pnumber
					 FROM Position
					 WHERE ename = 'University of New South Wales')
	LOOP
	UPDATE POSITION
	Set extras = extras || 'Rolls Royce'
	WHERE pnumber = Currlist.pnumber;
	END LOOP;
	
	For Nextlist IN (Select pnumber
					 FROM Position
					 WHERE extras LIKE '%mouse pad%')
	LOOP
	UPDATE Position
	Set extras = extras || 'computer'
	WHERE pnumber = Nextlist.pnumber;
	END LOOP;
END;
/
show errors