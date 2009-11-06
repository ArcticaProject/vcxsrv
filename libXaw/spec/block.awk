BEGIN {
	firstchar = "@";
	a["a"] = "A"; a["b"] = "B"; a["c"] = "C";
	a["d"] = "D"; a["e"] = "E"; a["f"] = "F";
	a["g"] = "G"; a["h"] = "H"; a["i"] = "I";
	a["j"] = "J"; a["k"] = "K"; a["l"] = "L";
	a["m"] = "M"; a["n"] = "N"; a["o"] = "O";
	a["p"] = "P"; a["q"] = "Q"; a["r"] = "R";
	a["s"] = "S"; a["t"] = "T"; a["u"] = "U";
	a["v"] = "V"; a["w"] = "W"; a["x"] = "X";
	a["y"] = "Y"; a["z"] = "Z";
}

{
	c = substr($2,2,1);
	if (c >= "a" && c <= "z")
		c = a[c];
	if (c != firstchar)
		printf(".LB %s\n", c);
	firstchar = c;
	print;
}
