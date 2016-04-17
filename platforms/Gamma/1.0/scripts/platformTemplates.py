# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
"""


class Templates(object):
    def __init__(self):
        self.stream_begin_code = '// Starting stream %02i -------------------------\n{\n'
        self.stream_end_code = '} // Stream End %02i\n'
        
        # Internal templates
        self.str_rate_begin_code = '{ // Start new rate %i\n' 
        self.str_rate_end_code = '\n}  // Close Rate %i\n' 
        self.str_assignment = '%s = %s;\n'
        self.str_increment = '%s += %s;\n'
        
        self.rate_stack = []
        self.rate_counter = 0
        self.domain_rate = 44100 # TODO set this from domain configuration
        pass
    
    def number_to_string(self, number):
        if type(number) == int:
            s = '%i;\n'%number
        elif type(number) == float:
            s = '%.8f;'%number
        else:
            raise ValueError(u"Unsupported type '%s' in assignment."%type(number).__name__)
        return s;
    
    def assignment(self, assignee, value):
        if not type(value) == str and not type(value) == unicode:
            value = self.number_to_string(value)
        code = self.str_assignment%(assignee, value)
        return code
        
    def increment(self, assignee, value):
        if not type(value) == str:
            value = self.number_to_string(value)
        code = self.str_increment%(assignee, value)
        return code
        
    def instantiation_code(self, instance):
        if instance['type'] == 'real':
            code = 'float ' + instance['handle'] + ';\n'
        elif instance['type'] =='bundle':
            if instance['bundletype'] == 'real':
                code = 'float ' + instance['handle'] + '[%i];\n'%instance['size']
            else:
                raise ValueError("Unsupported bundle type.")
        elif instance['type'] == 'module':
            code = instance['moduletype'] + ' ' + instance['handle'] + ';\n'
        elif instance['type'] == 'reaction':
            code = instance['reactiontype'] + ' ' + instance['handle'] + ';\n'
        else:
            raise ValueError('Unsupported type for instance')
        return code
                
    def initialization_code(self, instance):
        code = ''
        if instance['type'] == 'real':
            code = instance['handle'] + ' = ' + instance['code'] + ';\n'
        elif instance['type'] == 'block':
            code = ''
        else:
            ValueError("Unsupported type for initialization: " + instance['type'])
        return code
      
    # Handling of rate changes within a stream -------------------------------
    def rate_init_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            if rate < self.domain_rate:
                code = 'counter_%03i = 1.0;\n'%(index)
            else:
                code = 'counter_%03i = 0.0;\n'%(index)
        return code
        
    def rate_instance_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            code = 'float counter_%03i;\n'%(index)
        return code
        
    def rate_start(self, rate):
        self.rate_stack.append(rate)
        
    def rate_start_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            if rate < self.domain_rate:
                code += self.str_rate_begin_code%rate
                code += 'if (counter_%03i >= 1.0) {\ncounter_%03i -= 1.0;\n'%(index, index)
            else:
                code += self.str_rate_begin_code%rate
                code += 'while (counter_%03i < 1.0) {\n'%(index)
        return code
        
    def rate_stack_size(self):
        return len(self.rate_stack)
    
    def rate_end_code(self):
        if len(self.rate_stack) > 0:
            code = ''
            rate = self.rate_stack.pop()
            index = self.rate_counter
            if not rate == self.domain_rate:
                if rate < self.domain_rate:
                    code += '}\n' # Closes counter check above
                    code += templates.str_rate_end_code%rate
                    code += 'counter_%03i += %.10f;\n'%(index, float(rate)/self.domain_rate)
                else:
                    code += 'counter_%03i += %.10f;\n'%(index, self.domain_rate/ float(rate))
                    code += '}\n' # Closes counter check above
                    code += 'counter_%03i -= 1.0;\n'%(index)
                    code += templates.str_rate_end_code%rate
            self.rate_counter += 1
            return code
        else:
            return ''
        
    # Module code ------------------------------------------------------------
    def module_declaration(self, name, declaration_code, init_code, input_declaration, process_code):
        declaration = '''struct %s {
    %s %s() {
        %s
    }
    float process(%s) {
        %s
    }
};'''%(name, declaration_code, name, init_code, input_declaration, process_code)
        return declaration
        
    def module_processing_code(self, handle, in_tokens):
        if len(in_tokens) > 0:
            code = handle + '.process(' + in_tokens[0] + ')'
        else:
            code = handle + '.process()'
        return code
        
    def module_property_setter(self, name, block_name, prop_type):
        code = ''
        if prop_type == 'real':
            code += 'void set_' + name + '(float value) {\n'
            code += block_name + '= value;\n'
            code += '\n}\n'
        return code
        
    def module_output_code(self, output_block):
        code = ''        
        if output_block:
            code += 'return %s;\n'%(output_block['name']) 
        return code
        
    # Configuration code -----------------------------------------------------
    def get_config_code(self, sample_rate = 44100, block_size = 512,
                        num_out_chnls = 2, num_in_chnls = 2): 
        config_template_code = '''
    AudioDevice adevi = AudioDevice::defaultInput();
    AudioDevice adevo = AudioDevice::defaultOutput();
    //AudioDevice adevi = AudioDevice("firewire_pcm");
    //AudioDevice adevo = AudioDevice("firewire_pcm");
    
    //int maxOChans = adevo.channelsOutMax();
    //int maxIChans = adevi.channelsOutMax();
    //printf("Max input channels:  %d\\n", maxIChans);
    //printf("Max output channels: %d\\n", maxOChans);
    
    // To open the maximum number of channels, we can hand in the queried values...
    //AudioIO io(256, 44100., audioCB, NULL, maxOChans, maxIChans);
    
    // ... or just use -1
    
    AudioIO io(%%block_size%%, %%sample_rate%%, audioCB, NULL, %%num_out_chnls%%, %%num_in_chnls%%);
        '''
        config_template_code = config_template_code.replace("%%block_size%%", str(block_size))
        config_template_code = config_template_code.replace("%%sample_rate%%", str(sample_rate))
        config_template_code = config_template_code.replace("%%num_out_chnls%%", str(num_out_chnls))
        config_template_code = config_template_code.replace("%%num_in_chnls%%", str(num_in_chnls))
        return config_template_code

templates = Templates()