SPOOL task2
SET ECHO ON
SET FEEDBACK ON
SET LINESIZE 100
SET PAGESIZE 100
SET SERVEROUTPUT ON


CREATE OR REPLACE FUNCTION FINDAPPS(Pos_num NUMBER) RETURN VARCHAR IS FoundApps varchar(300);
BEGIN
	FoundApps := ': ';
	FOR curr_list IN (SELECT E.anumber
					  FROM APPLICANT E
					  WHERE EXISTS(SELECT *
								   FROM SPOSSESSED L
							       WHERE EXISTS(SELECT R.sname
												FROM SNEEDED R
												WHERE (R.pnumber = Pos_num) AND (L.sname = R.sname) AND (E.anumber = L.anumber)
												)
									)
					  )
	
	LOOP
	FoundApps := FoundApps || LPAD(curr_list.anumber, 8, '0') || ' ';
	END LOOP;
	Return FoundApps;
END;
/

SELECT LPAD(pnumber, 8, '0'), FINDAPPS(pnumber)
FROM POSITION
WHERE pnumber = 00000001;

SELECT LPAD(pnumber, 8, '0'), FINDAPPS(pnumber)
FROM POSITION
WHERE pnumber = 00000010;

SPOOL OFF
