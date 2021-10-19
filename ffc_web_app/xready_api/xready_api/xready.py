
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
        self.cgra_name = self.json_prj['cgra_name']
        self.compile_flags = self.json_prj['compile_flags']
        self.prj_path = ''
        self.run_mode = self.json_prj['run_mode']
        self.log = ''
        self.read_cgra_db(self.cgra_name)

    def read_cgra_db(self,name):
        FFC_CGRA_DB = os.environ['FFC_CGRA_DB']
        with open(os.path.join(FFC_CGRA_DB,'cgra_db.json')) as f:
            cgra_db = json.load(f)
            self.cgra = cgra_db[ self.cgra_name]


    def create_prj(self):
        try:
            FFC_PRJ_DIR = os.environ['FFC_PRJ_DIR']
            FFC_XREADY_API = os.environ['FFC_XREADY_API']
            self.prj_path =  os.path.join(FFC_PRJ_DIR, self.prj_name)

            prj_host = os.path.join(self.prj_path,'host')
            prj_src = os.path.join(self.prj_path,'host/src')
            template_prj = os.path.join(FFC_XREADY_API,'template.prj')

            if not os.path.exists(self.prj_path):
                sp.run(['cp','-r',template_prj,self.prj_path])

            for s in self.sources:
                with open(os.path.join(prj_src,s),'w') as f:
                    f.write(self.sources[s])

            with open(os.path.join(prj_host,'%s.json'%(self.df['name'])),'w') as f:
                f.write(json.dumps(self.df,indent=4, sort_keys=True))

            with open(os.path.join(prj_host,'cgra_arch.json'),'w') as f:
                f.write(json.dumps(self.cgra["arch"],indent=4, sort_keys=True))

            with open(os.path.join(self.prj_path,'prj.json'),'w') as f:
                f.write(json.dumps(self.json_prj,indent=4, sort_keys=True))

        except Exception as e:
            self.log = str(e) + '\n'
            return False

        return True

    def compile_lib(self):
        try:
            os.chdir(os.path.join(self.prj_path,'lib'))
            r = sp.run(['make','-j4'],stdout=sp.PIPE,stderr=sp.PIPE)
            if r.returncode == 0:
                return True
            else:
                self.log += 'ERROR:\n' + r.stderr.decode(encoding='UTF-8',errors='strict') + '\n'
                return False
        except Exception as e:
            self.log += str(e) + '\n'
            return False

    def compile_and_exec_host(self):
        try:
            prj_host = os.path.join(self.prj_path,'host')
            os.chdir(prj_host)
            if self.run_mode == 'sim':
                args_args = ['sim','KERNEL_NAME=%s'%self.cgra['kernel_name'],'SIM_XLCBIN=%s'%self.cgra['sim_xclbin']]
            elif self.run_mode == 'cgra':
                args_args = ['cgra','KERNEL_NAME=%s'%self.cgra['kernel_name'],'HW_XLCBIN=%s'%self.cgra['hw_xclbin']]
            else:
                args_args = ['cpu']
            args = ['make'] + args_args

            r = sp.run(args,stdout=sp.PIPE,stderr=sp.PIPE)
            self.log += r.stdout.decode(encoding='UTF-8',errors='strict') + '\n'
            self.log += r.stderr.decode(encoding='UTF-8',errors='strict') + '\n'
            if r.returncode == 0:
                return True
            else:
                return False
        except Exception as e:
                self.log = str(e) + '\n'

    def execute(self):
        if(self.create_prj()):
            if(self.compile_lib() and self.compile_and_exec_host()):
               pass

        return self.log

