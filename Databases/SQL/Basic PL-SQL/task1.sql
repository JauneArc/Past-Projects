SPOOL task1
SET ECHO ON
SET FEEDBACK ON
SET LINESIZE 100
SET PAGESIZE 100
SET SERVEROUTPUT ON


CREATE OR REPLACE PROCEDURE LISTSKILL AS userlist varchar(700);
BEGIN
	FOR Curr_list IN (SELECT E.fname, E.lname, S.sname, S.slevel 
					  FROM APPLICANT E LEFT OUTER JOIN SPOSSESSED S 
					  ON S.anumber = E.anumber)
	LOOP
	dbms_output.put_line(Curr_list.fname || ' ' || Curr_list.lname || ': ' || Curr_list.sname || ' ' || Curr_list.slevel||', ' || chr(13)||chr(10));
	END LOOP;
END;

/

SPOOL OFF