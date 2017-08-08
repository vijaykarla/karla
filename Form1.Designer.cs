namespace ExcelCompare
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnRNfile = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.bntCAFile = new System.Windows.Forms.Button();
            this.openFileDialog2 = new System.Windows.Forms.OpenFileDialog();
            this.btnCompare = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnRNfile
            // 
            this.btnRNfile.Location = new System.Drawing.Point(37, 68);
            this.btnRNfile.Name = "btnRNfile";
            this.btnRNfile.Size = new System.Drawing.Size(75, 23);
            this.btnRNfile.TabIndex = 0;
            this.btnRNfile.Text = "RN File";
            this.btnRNfile.UseVisualStyleBackColor = true;
            this.btnRNfile.Click += new System.EventHandler(this.btnRN_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.FileName = "openFileDialog1";
            this.openFileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // bntCAFile
            // 
            this.bntCAFile.Location = new System.Drawing.Point(166, 68);
            this.bntCAFile.Name = "bntCAFile";
            this.bntCAFile.Size = new System.Drawing.Size(75, 23);
            this.bntCAFile.TabIndex = 1;
            this.bntCAFile.Text = "CA File";
            this.bntCAFile.UseVisualStyleBackColor = true;
            this.bntCAFile.Click += new System.EventHandler(this.btnCA_Click);
            // 
            // openFileDialog2
            // 
            this.openFileDialog2.FileName = "openFileDialog1";
            this.openFileDialog2.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog2_FileOk);
            // 
            // btnCompare
            // 
            this.btnCompare.Location = new System.Drawing.Point(96, 153);
            this.btnCompare.Name = "btnCompare";
            this.btnCompare.Size = new System.Drawing.Size(75, 23);
            this.btnCompare.TabIndex = 2;
            this.btnCompare.Text = "Compare";
            this.btnCompare.UseVisualStyleBackColor = true;
            this.btnCompare.Click += new System.EventHandler(this.btnCompare_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.btnCompare);
            this.Controls.Add(this.bntCAFile);
            this.Controls.Add(this.btnRNfile);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnRNfile;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Button bntCAFile;
        private System.Windows.Forms.OpenFileDialog openFileDialog2;
        private System.Windows.Forms.Button btnCompare;

    }
}

