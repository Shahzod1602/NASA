import unittest, tempfile, subprocess, sys, json
from pathlib import Path

class ProfileSummaryTests(unittest.TestCase):
    def run_summary(self, values, *args):
        with tempfile.TemporaryDirectory() as tmp:
            source=Path(tmp)/'capture.csv'; output=Path(tmp)/'summary.json'
            source.write_text('FrameTime,EVENTS\n'+''.join(f'{v},\n' for v in values)+'FrameTime,EVENTS\n',encoding='utf-8')
            result=subprocess.run([sys.executable,str(Path(__file__).with_name('summarize_profile.py')),str(source),'--warmup','0','--expected-frames','100','--output',str(output),*args],capture_output=True,text=True)
            return result, json.loads(output.read_text()) if output.exists() else None
    def test_constant_and_visible_label(self):
        result,data=self.run_summary([20]*100,'--onscreen')
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(data['mean_fps'],50)
        self.assertEqual(data['one_percent_low_fps'],50)
        self.assertTrue(data['limitations'].startswith('On-screen'))
    def test_hitch_is_not_hidden_by_mean(self):
        result,data=self.run_summary([10]*99+[100])
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(data['one_percent_low_fps'],10)
        self.assertEqual(data['largest_frame_ms'],100)
        self.assertEqual(data['p99_frame_ms'],10)
    def test_incomplete_capture_rejected(self):
        result,data=self.run_summary([20]*99)
        self.assertNotEqual(result.returncode,0)
        self.assertIsNone(data)
    def test_invalid_frames_not_counted(self):
        result,data=self.run_summary([20]*97+['nan','inf',-1])
        self.assertNotEqual(result.returncode,0)
        self.assertIsNone(data)

if __name__=='__main__': unittest.main()
