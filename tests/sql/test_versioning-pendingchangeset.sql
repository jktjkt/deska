BEGIN;

SET search_path TO pgtap, api, genproc, deska, history, production;


CREATE TABLE public.pending_changeset(
	 chid text,
	 chstatus deska.changeset_status
);

INSERT INTO public.pending_changeset VALUES ('tmpA','INPROGRESS');
INSERT INTO public.pending_changeset VALUES ('tmpB','DETACHED');

CREATE OR REPLACE FUNCTION pgtap.test_pending_changeset1()
RETURNS SETOF TEXT AS
$$
DECLARE
	 first_changeset text;
BEGIN
	RETURN NEXT throws_ok( 'PERFORM vendor_add(''DELL'')' );
	PERFORM startchangeset();
	first_changeset = id2changeset(my_version());
	UPDATE public.pending_changeset SET chid = first_changeset WHERE chid = 'tmpA';
	PERFORM detachfromcurrentchangeset('detach message tmpA');

	PERFORM startchangeset();
	UPDATE public.pending_changeset SET chid = id2changeset(my_version()) WHERE chid = 'tmpB';
	PERFORM detachfromcurrentchangeset('detach message tmpB');
	PERFORM resumechangeset(first_changeset);

	RETURN NEXT results_eq('SELECT changeset, status FROM pendingchangesets() ORDER BY changeset','SELECT * FROM public.pending_changeset ORDER BY chid','detached and in progress changestets are correct');

	RETURN NEXT throws_ok( 'PERFORM startchangeset()' );
END;
$$
LANGUAGE plpgsql; 


SELECT * FROM runtests();

ROLLBACK;  