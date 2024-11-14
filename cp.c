[^ \t\n;][^ \t\n;]* {  
  //Assume that file names have only alpha chars 
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}   
