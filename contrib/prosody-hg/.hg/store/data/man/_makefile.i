         C   B      x����������=�S|"�8ʂ��~�s�|            uall: prosodyctl.man

%.man: %.markdown
	pandoc -s -t man -o $@ $^
