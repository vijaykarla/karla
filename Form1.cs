using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Microsoft.Office.Interop.Excel;
using System.Runtime.InteropServices;
using System.IO;

namespace ExcelCompare
{
    public partial class Form1 : Form
    {
        int RNFSheet = 2;
        int RNFileColumn = 4;
        int RNPathColumn = 7;
        int RNVersionColumn = 6;

        int RCASheet = 1;
        int CAFileColumn = 1;
        int CAPathColumn = 1;
        int CAVersionColumn = 1;

        string RNFile = string.Empty;
        string CAFile = string.Empty;
        public Form1()
        {
            InitializeComponent();
        }

        private void btnRN_Click(object sender, EventArgs e)
        {
            DialogResult result = openFileDialog1.ShowDialog();
            if (result == DialogResult.OK) // Test result.
            {
            }
            Console.WriteLine(result); // <-- For d
        }
        
        private void btnCA_Click(object sender, EventArgs e)
        {
            DialogResult result = openFileDialog2.ShowDialog();
            if (result == DialogResult.OK) // Test result.
            {
            }
            Console.WriteLine(result); // <-- For d
        }

        private void openFileDialog1_FileOk(object sender, CancelEventArgs e)
        {
            RNFile = openFileDialog1.FileName;
        }
      

        private void openFileDialog2_FileOk(object sender, CancelEventArgs e)
        {
            CAFile = openFileDialog2.FileName;
        }

        private void btnCompare_Click(object sender, EventArgs e)
        {
            CompareFiles();
        }

        public void CompareFiles()
        {
            List<FileItem> RNItems = new List<FileItem>();

            Microsoft.Office.Interop.Excel.Application _excelApp = new Microsoft.Office.Interop.Excel.Application();
            _excelApp.Visible = false;           

            //open the workbook
            Workbook workbook = _excelApp.Workbooks.Open(RNFile,
                Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Type.Missing, Type.Missing);

            //select the first sheet        
            Worksheet worksheet = (Worksheet)workbook.Worksheets[RNFSheet];

            //find the used range in worksheet
            Range excelRange = worksheet.UsedRange;

            //get an object array of all of the cells in the worksheet (their values)
            object[,] valueArray = (object[,])excelRange.get_Value(
                        XlRangeValueDataType.xlRangeValueDefault);

            //access the cells
           
            for (int row = 1; row <= worksheet.UsedRange.Rows.Count; ++row)
            {
                if (valueArray[row, RNFileColumn] != null && !string.IsNullOrWhiteSpace(valueArray[row, RNFileColumn].ToString()))
                {
                    string fileName = valueArray[row, RNFileColumn].ToString();
                    if (fileName.IndexOfAny(Path.GetInvalidFileNameChars()) < 0 && fileName.Contains("."))
                    {
                        FileItem RNItem = new FileItem();
                        RNItem.FileName = fileName;
                        if (valueArray[row, RNPathColumn] != null && !string.IsNullOrWhiteSpace(valueArray[row, RNPathColumn].ToString()))
                        {
                            RNItem.FilePath = valueArray[row, RNPathColumn].ToString();
                        }
                        if (valueArray[row, RNVersionColumn] != null && !string.IsNullOrWhiteSpace(valueArray[row, RNVersionColumn].ToString()))
                        {
                            RNItem.FileVersion = valueArray[row, RNVersionColumn].ToString();
                        }
                        int colorNumber = System.Convert.ToInt32(((Range)worksheet.Cells[row, RNFileColumn]).Interior.Color);
                        Color color = System.Drawing.ColorTranslator.FromOle(colorNumber);
                        RNItem.CellColor = color;
                        RNItems.Add(RNItem);
                    }
                }
               
            }

            //clean up stuffs
            workbook.Close(false, Type.Missing, Type.Missing);
            Marshal.ReleaseComObject(workbook);

            _excelApp.Quit();
            Marshal.FinalReleaseComObject(_excelApp);

            //======================================================================

            _excelApp = new Microsoft.Office.Interop.Excel.Application();
            _excelApp.Visible = false;

            //open the workbook
            workbook = _excelApp.Workbooks.Open(RNFile,
                Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Type.Missing, Type.Missing, Type.Missing, Type.Missing,
                Type.Missing, Type.Missing);

            //select the first sheet        
            worksheet = (Worksheet)workbook.Worksheets[2];

            //find the used range in worksheet
            excelRange = worksheet.UsedRange;

            //get an object array of all of the cells in the worksheet (their values)
            valueArray = (object[,])excelRange.get_Value(
                        XlRangeValueDataType.xlRangeValueDefault);

            //access the cells

            for (int row = 1; row <= worksheet.UsedRange.Rows.Count; ++row)
            {
                if (valueArray[row, 4] != null && !string.IsNullOrWhiteSpace(valueArray[row, 4].ToString()))
                {
                    string fileName = valueArray[row, 4].ToString();
                    if (fileName.IndexOfAny(Path.GetInvalidFileNameChars()) < 0 && fileName.Contains("."))
                    {
                        FileItem RNItem = new FileItem();
                        RNItem.FileName = fileName;
                        if (valueArray[row, 7] != null && !string.IsNullOrWhiteSpace(valueArray[row, 7].ToString()))
                        {
                            RNItem.FilePath = valueArray[row, 7].ToString();
                        }
                        if (valueArray[row, 6] != null && !string.IsNullOrWhiteSpace(valueArray[row, 6].ToString()))
                        {
                            RNItem.FileVersion = valueArray[row, 6].ToString();
                        }
                        int colorNumber = System.Convert.ToInt32(((Range)worksheet.Cells[row, 4]).Interior.Color);
                        Color color = System.Drawing.ColorTranslator.FromOle(colorNumber);
                        RNItem.CellColor = color;
                        RNItems.Add(RNItem);
                    }
                }

            }

            //clean up stuffs
            workbook.Close(false, Type.Missing, Type.Missing);
            Marshal.ReleaseComObject(workbook);

            _excelApp.Quit();
            Marshal.FinalReleaseComObject(_excelApp);
        }
    }

    public class FileItem
    {
        public string FileName { get; set; }
        public string FilePath { get; set; }
        public string FileVersion { get; set; }
        public Color CellColor { get; set; }
    }
}
