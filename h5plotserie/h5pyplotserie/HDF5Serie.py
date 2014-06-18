import h5py
import numpy
import warnings
import re

class HDF5Serie(h5py.File):

    def addGroupWithData(self, parent, groupname, labels, data):
        self.addGroup(groupname, None, parent)
        self.addData(parent[groupname], labels, data)
    
    def addGroup(self, groupname, description=None, parent=None, overwrite=False):
        '''
        Add a group to the given group
        '''
        if parent is None:
            parent = self
        
        if groupname in parent:
            if overwrite == True:
                del parent[groupname]
            else:
                raise Exception('Group "' + groupname + '" already exists.')
        
        parent.create_group(groupname)
        
        if description is not None:
            self.addAttribute(parent[groupname], 'Description', description)
            
        return parent[groupname]
    
    def addData(self, parent, labels, dat, description=None):
        '''
        Add Data to the given group
        parent: group to add data to
        labels: list of labels
        dat: data-matrix as numpy array
        '''
        if 'data' in parent:
            raise Exception('Dataset "data" already exists.')
        
        if len(labels) != len(dat[0]):
            raise Exception('Length of labels-vector does not fit to length of data-array!')
        
        ds = parent.create_dataset('data', data=dat, maxshape=[None, len(labels)])
        
        self.addAttribute(parent['data'], 'Column Label', labels)
        
        
        if description is not None:
            self.addAttribute(parent['data'], 'Description', description)
        
    def addAttribute(self, parent, attributeName, dat):
        '''
        Adds an attribute to the given parent
        '''
        parent.attrs.create(attributeName, data=dat, dtype = h5py.special_dtype(vlen=str))
        
    def walk(self, parent = None):
        '''
        searches the tree of a parent object
        '''
        def subwalk(subgroup):
            try:
                retList.append((subgroup, parent[subgroup].keys()))
                for item in parent[subgroup]:
                    subwalk(subgroup + item + '/')
            except:
                pass
        
        if parent is None:
            parent = self
            
        retList = []
        retList.append((parent.name, parent.keys()))
        for group in parent:
            if not group == 'data':
                 subwalk(parent.name + group + '/')
                 
        return retList

    def findAllGroups(self, regexp, parent = None):
        '''
        Copied from http://stackoverflow.com/questions/4639506/os-walk-with-regex on 27.01.2014
        Generator yielding all files under `dirpath` whose absolute path
        matches the regular expression `regexp`.
        Usage:
        >>> for filename in iter_matching('/', r'/home.*\.bak'):
            ....    # do something
         '''
        retList = []
        for parentGroup, childGroups in self.walk(parent):
            if parentGroup[:-1] in retList:
                break
            else:
                for child in childGroups:
                    abspath = ''.join([parentGroup, child])
                    if re.search(regexp, child):
                        retList.append(abspath)
        return retList
                        
    def getNbCols(self, path, group=None):
        data = self.getData(path, group)
        
        if data is not None:
            return len(data.T)
        else:
            return None
        
    def getNbRows(self, path, group=None):
        data = self.getData(path, group)
        
        if data is not None:
            return len(data)
        else:
            return None
        
    def getAttribute(self, attribute, path, group=None):
        newGroup = self.getGroup(path, group)
        
        if newGroup is not None:
            if attribute in newGroup.attrs: 
                return newGroup.attrs[attribute]
            else:
                return None
        else: 
            return None   
    
    def getColumnLabels(self, path, group=None):
        labels = self.getAttribute('Column Label', path, group)
        
        if labels is not None:
            return list(labels)
        
    def getData(self, path=None, group=None):
            
        if path is not None:
            group = self.getGroup(path, group)
        
        if group is None:
            return group
        else:
            if path.endswith('data'):
                return group.value
            elif 'data' in group:
                return group['data'].value
            else:   
                return None
            
    def getDataVec(self, identifier, path=None, group=None):
        if path is not None:
            group = self.getGroup(path, group)
        
        if group is None:
            return group
        else:
            if 'data' in group:
                try:
                    labels = group['data'].attrs['Column Label']
                    ind = list(labels).index(identifier)
                    return group['data'].value[:,ind]
                except:
                    return None
            else:   
                return None
        
    
    def checkGroup(self, path, group=None):
        if group is None:
            group = self
            
        for subgroup in path.split("/"):
            if subgroup in group:
                group = group[subgroup]
            else:
                return False
            
        return True
            
        return group
    
    def getGroup(self, path, group=None):
        if group is None:
            group = self
            
        if path.startswith('/'):
            path = path[1:]
            
        for subgroup in path.split("/"):
            if subgroup in group:
                group = group[subgroup]
            else:
                warnings.warn("No Group named " + subgroup)
                return None
            
        return group
    
    def printGroups(self, parent=None, indent=None):
        if parent is None:
            parent = self
            indent = ""
            
        for group in parent:
            if group != "data":
                print indent + group
                self.printGroups(parent[group], indent+"    ")
                
            else:
                self.printLabels(parent, indent)
                
    def printLabels(self, parent, indent):
        for item in parent['data'].attrs['Column Label']:
            print indent + "-" +  item