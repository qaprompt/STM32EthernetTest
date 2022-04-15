import os,gzip,json,htmlmin
from jsmin import jsmin
from string import Template

from cachetools import RRCache

TMPLT_FILE = "./index_tmplt.c"
OUT_FILE = "../Core/Src/networking/index.c"
src_file_list = []

class FileContents():
    def __init__(self, source):

        self.sourceFile = source
        self.url = "/" + self.sourceFile.strip()
        self.codeName = self.sourceFile.replace(".", "_")

        # Hacky way to define mime type
        tmp = os.path.splitext(source)
        self.mimeType = tmp[1].replace(".", "")

        with open(self.sourceFile, "r") as infile:
            fileContents = infile.read()

            if(self.mimeType=="html"):
                minContents = htmlmin.minify(fileContents, remove_comments=True, remove_empty_space=True)
            elif(self.mimeType=="css"):
                minContents = fileContents.replace("\n\n", "\n").replace("    ", "")
            elif(self.mimeType=="js"):
                minContents = jsmin(fileContents)
            else:
                minContents = fileContents

            self.fileContentsStr = json.dumps(minContents)

    def getDeclaration(self):
        return "static const char {}[] = {};".format(self.codeName, self.fileContentsStr)



if __name__ == "__main__":

    # Find all files to include
    for file in os.listdir("."):
        if file.endswith(".html") or file.endswith(".js") or file.endswith(".css"):
            print("Adding {}".format(file))
            src_file_list.append(FileContents(file))

    with open(OUT_FILE, "w") as outfile:
        pageData = ""
        for file in src_file_list:
            pageData += (file.getDeclaration() + "\n\n")

        switchyard = ""

        #Hardcode the homepage
        switchyard += "if(mg_http_match_uri(hm, \"/\")) {\n"
        switchyard += "      mg_http_reply(c, 200,  header_html,  index_html );\n"
        switchyard += "      printf(\"[WEBSERVER] Served /\\n\");\n"
        switchyard += "   } "

        # Handle the other files too
        for file in src_file_list:
            switchyard += "else if(mg_http_match_uri(hm, \"{}\")) {{\n".format(file.url)
            switchyard += "      mg_http_reply(c, 200,  header_{},  {} );\n".format(file.mimeType ,file.codeName)
            switchyard += "      printf(\"[WEBSERVER] Served {}\\n\");\n".format(file.url)
            switchyard += "   } "

        # Hardcode the 404 page
        switchyard += "else {\n"
        switchyard += "      //URL Not found\n"
        switchyard += "      printf(\"[WEBSERVER] Could not find requested resource!\\n\");\n"
        switchyard += "      mg_http_reply(c, 404,  header_html,  FourOhFour_html );\n".format(file.mimeType ,file.codeName)
        switchyard += "   } \n\n"


        with open(TMPLT_FILE, "r") as tmpltFile:
            tmplt = Template(tmpltFile.read())
            outfile.write(tmplt.substitute(pageData = pageData, pageSwitchyard = switchyard))
        

            

