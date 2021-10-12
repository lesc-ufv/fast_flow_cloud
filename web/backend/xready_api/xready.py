
import sys
import os
import json
import subprocess as sp
import shutil

class XReady:
    def __init__(self, json_prj):
        self.json_prj = json_prj
        self.prj_name =self.json_prj['project_name']
        self.sources = self.json_prj['sources']
        self.df = self.json_prj['dataflows'][0]
        self.cgra_arch = self.json_prj['cgra_arch']
        self.compile_flags = self.json_prj['compile_flags']
        self.prj_path = ''

    def create_prj(self):
        try:
            root = os.environ['XREADY_FFC_PRJ_TMP']
            self.prj_path =  os.path.join(root, self.prj_name)
            try:
                shutil.rmtree(self.prj_path)
            except:
                pass

            os.mkdir(self.prj_path)
            for s in self.sources:
                with open(os.path.join(self.prj_path,s),'w') as f:
                    f.write(self.sources[s])
            with open(os.path.join(self.prj_path,'%s.json'%(self.df['name'])),'w') as f:
                f.write(json.dumps(self.df))
            with open(os.path.join(self.prj_path,'cgra_arch.json'),'w') as f:
                f.write(json.dumps(self.cgra_arch))
        except Exception as e:
            return str(e)

        return "project created with success!"


    def compile_prj(self):
        os.chdir(self.prj_path)
        args = ['g++']+self.compile_flags+list(self.sources.keys())
        r = sp.run(args,capture_output=True, text=True)
        if r.returncode == 0:
            r = str(r.stdout)
        else:
            r = 'Build error:' + str(r.stderr)
        return r
    def execute(self):
        r1 = self.create_prj()
        r2 = self.compile_prj()
        return '%s\n%s'%(r1,r2)

