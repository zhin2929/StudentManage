
// StudentInfoClientDlg.h: 头文件
//

#pragma once
#include "QUDPClient.h"
#include <functional>

// CStudentInfoClientDlg 对话框
class CStudentInfoClientDlg : public CDialogEx
{
	

// 构造
public:
	CStudentInfoClientDlg(CWnd* pParent = nullptr);	// 标准构造函数
	~CStudentInfoClientDlg();	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STUDENTINFOCLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_treeDB;
	CListCtrl m_listCtrlData;
	QUDPClient m_qudp;
	FILE* pFile;
	fpos_t nFileSize;
	afx_msg void OnBnClickedOk();

	UpdateUIFunc m_updateUIFunc;
  void UpdateUI2(QUDP_TYPE type, size_t nLen, std::vector<char>& dataBuf);
	void UpdateUI(QUDP_TYPE type, char* packetBuf);
	CString m_editTest;
	std::vector<std::string> m_vctDb;
	afx_msg void OnDblclkTreeDb(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListData(NMHDR* pNMHDR, LRESULT* pResult);

	CString m_currentTable;
	CString m_currentDb;
	CString m_szClassId;
	CString m_szClassName;
	CString m_szCourseId;
	CString m_szCourseName;
	CString m_szScoreCourseId;
	CString m_szScoreStuId;
	CString m_szScoreNum;
	CString m_szStuClassId;
	CString m_szStuId;
	CString m_szStuName;
	afx_msg void OnBnClickedClassInsert();
	afx_msg void OnBnClickedClassDelete();
	afx_msg void OnBnClickedClassUpdate();
	afx_msg void OnBnClickedClassSelect();
	afx_msg void OnBnClickedCourseInsert();
	afx_msg void OnBnClickedCourseDelete();
	afx_msg void OnBnClickedCourseUpdate();
	afx_msg void OnBnClickedCourseSelect();
	afx_msg void OnBnClickedScoreInsert();
	afx_msg void OnBnClickedScoreDelete();
	afx_msg void OnBnClickedScoreUpdate();
	afx_msg void OnBnClickedScoreSelect();
	afx_msg void OnBnClickedStuInsert();
	afx_msg void OnBnClickedStuDelete();
	afx_msg void OnBnClickedStuUpdate();
	afx_msg void OnBnClickedStuSelect();

	void ClearListCtrl();
	BOOL m_checkClassId;
	BOOL m_checkClassName;
	BOOL m_checkCourseId;
	BOOL m_checkCourseName;
	BOOL m_checkStuId;
	BOOL m_checkStuName;
	BOOL m_checkCourseNum;
	afx_msg void OnBnClickedQuery();
	int m_nCount = 0;
	int m_nStudentId;
	int m_nSelectItem;
	afx_msg void OnDelete();
	afx_msg void OnRclickListData(NMHDR* pNMHDR, LRESULT* pResult);
};
