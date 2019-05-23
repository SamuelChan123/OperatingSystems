package edu.duke.raft;

import java.util.Timer;
import java.util.TimerTask;

public class FollowerMode extends RaftMode {

  protected Timer t;
  // protected int votedFor = -1;
  // protected long timeout = ((long)Math.random()*ELECTION_TIMEOUT_MIN + ELECTION_TIMEOUT_MIN); 

  public void go () {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      System.out.println ("S" + 
			  mID + 
			  "." + 
			  term + 
			  ": switched to follower mode.");
      long timeout = getTimeout();
      // System.out.println("The timeout for S"+mID+"."+term+" is "+(int)timeout);
      t = scheduleTimer(timeout, mID);
    }
  }
  
  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      // int votedFor = mConfig.getVotedFor();
      int vote = term;

      //Reset election timeout timer
      t.cancel();
      long timeout = getTimeout();
      // System.out.println("The timeout (in requestVote) for S"+mID+"."+term+" is "+(int)timeout);
      t = scheduleTimer(timeout, mID);

      if (candidateTerm < term) {
        return vote;
      }

      int votedFor = mConfig.getVotedFor();
      if (candidateTerm >= term) {
        mConfig.setCurrentTerm(candidateTerm, 0);
        if(votedFor == candidateID || votedFor == 0){
          if(lastLogTerm > mLog.getLastTerm() || (lastLogIndex >= mLog.getLastIndex() && lastLogTerm == mLog.getLastTerm())){
            // System.out.println("We made it bruh!");
            mConfig.setCurrentTerm(candidateTerm, candidateID);
            // System.out.println(mConfig);
            // votedFor = candidateID;
            return 0;
          }
        }
      }
      return mConfig.getCurrentTerm();
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      int result = term;

      t.cancel();
      long timeout = getTimeout();
      // System.out.println("The timeout (in appendEntries) for S"+mID+"."+term+" is "+(int)timeout);
      t = scheduleTimer(timeout,mID);

      // System.out.println("(Before) Follower S"+mID+"."+term+" log: "+mLog);

      // System.out.println("Follower S"+mID+"."+term+" received log entry: "+entries);
      // System.out.println("Follower S"+mID+"."+term+" last log index: "+mLog.getLastIndex()+" and last log term: "+mLog.getLastTerm());
      // System.out.println("Leader S"+mID+"."+term+" prev log index: "+prevLogIndex+" and prev log term: "+prevLogTerm);

      //Leader isn't legitimate because of lower term
      if (leaderTerm < term) {
        // RaftServerImpl.setMode(new CandidateMode());
        return term;
      }

      mConfig.setCurrentTerm(leaderTerm,0);
      // term=mConfig.getCurrentTerm();
      //Check if you are able to append the entries. If heartbeat no need to check, just return 0;
      if(entries != null) {
        // System.out.println("First append check(follower) S"+mID+"."+term);
        // mConfig.setCurrentTerm(leaderTerm,0);
        // if (prevLogIndex != mLog.getLastIndex() || prevLogTerm != mLog.getLastTerm()){
        //   // mLog.insert(entries,prevLogIndex, prevLogTerm);
        //   // return 0;
        //   return term;
        // }
        if (prevLogIndex > mLog.getLastIndex() || (!(prevLogIndex <= -1) && prevLogTerm != mLog.getEntry(prevLogIndex).term)){
          // mLog.insert(entries,prevLogIndex, prevLogTerm);
          // System.out.println("Append failed!(follower)S"+mID+"."+term);
          // return 0;
          return mConfig.getCurrentTerm();
        }
        else{
          // System.out.println("We appendin'!(follower) S"+mID+"."+term);
          if (mLog.insert(entries,prevLogIndex,prevLogTerm) == -1) {
            return mConfig.getCurrentTerm();
          }  
          mCommitIndex = leaderCommit;
          // System.out.println("(After) Follower S"+mID+"."+term+" log: "+mLog);
          return 0;
        }
      }
      return 0;
    }
  }  

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      // System.out.println("S"+mID+" timed out!");
      if (timerID == mID) {
        t.cancel();
        //Handle
        // t.cancel();
        RaftServerImpl.setMode(new CandidateMode());

        // RaftServerImpl.setMode(new CandidateMode());
      }
      else {
        throw new RuntimeException("Oops I didn't set this timer!");
      }
    }
  }

  public long getTimeout() {
    long override = (long)mConfig.getTimeoutOverride();
    if (override <= 0){
      return (long)(Math.random()*ELECTION_TIMEOUT_MIN + ELECTION_TIMEOUT_MIN);
    } 
    return override;
  } 
}
  

// mConfig.setCurrentTerm((mConfig.getCurrentTerm() + 1), mID);
        // cm.initializeServer(mConfig,mLog,mLastApplied,mRmiPort,mID);


