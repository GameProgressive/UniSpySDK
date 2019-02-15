# RetroSpy SDK Makefile for Revolution platform

default:
	$(MAKE) -C common -f revolution.mak
	$(MAKE) -C ghttp -f revolution.mak
	$(MAKE) -C gt2 -f revolution.mak
	$(MAKE) -C qr2 -f revolution.mak
	$(MAKE) -C webservices -f revolution.mak
	$(MAKE) -C GP -f revolution.mak
	$(MAKE) -C gstats -f revolution.mak
	$(MAKE) -C natneg -f revolution.mak
	$(MAKE) -C pt -f revolution.mak
	$(MAKE) -C sake -f revolution.mak
	$(MAKE) -C gcdkey -f revolution.mak
	$(MAKE) -C brigades -f revolution.mak
	$(MAKE) -C Peer -f revolution.mak
	$(MAKE) -C Chat -f revolution.mak
	$(MAKE) -C sc -f revolution.mak
	$(MAKE) -C serverbrowsing -f revolution.mak
	$(MAKE) -C Voice2 -f revolution.mak

clean:
	$(MAKE) -C common -f revolution.mak clean
	$(MAKE) -C ghttp -f revolution.mak clean
	$(MAKE) -C gt2 -f revolution.mak clean
	$(MAKE) -C qr2 -f revolution.mak clean
	$(MAKE) -C webservices -f revolution.mak clean
	$(MAKE) -C GP -f revolution.mak clean
	$(MAKE) -C gstats -f revolution.mak clean
	$(MAKE) -C natneg -f revolution.mak clean
	$(MAKE) -C pt -f revolution.mak clean
	$(MAKE) -C sake -f revolution.mak clean
	$(MAKE) -C gcdkey -f revolution.mak clean
	$(MAKE) -C brigades -f revolution.mak clean
	$(MAKE) -C Peer -f revolution.mak clean
	$(MAKE) -C Chat -f revolution.mak clean
	$(MAKE) -C sc -f revolution.mak clean
	$(MAKE) -C serverbrowsing -f revolution.mak clean
	$(MAKE) -C Voice2 -f revolution.mak clean

	